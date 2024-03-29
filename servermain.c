#include "serverfunctions.h"
#include <signal.h>

#define BUF 1024
#define IPLOCK 120
#define PATHLOCK "lockedclients"

short port = 0;
char *spool = NULL;

struct host
{
	char *ip;
	time_t time;
	struct host *next;
};

typedef struct
{
	struct sockaddr_in address;
	int socket;
} args;

struct host *locked = NULL;

const char *options[] = { "quit", "send", "list", "read", "del", "login" };

int findcom(char s[BUF])
{
	int i;

	for(i = 0; i < 4; i++)
	{
		if(strncmp(s, options[i], 4) == 0)
			return i;
	}

	if(strncmp(s, options[4], 3) == 0)
			return 4;

	if(strncmp(s, options[5], 5) == 0)
		return 5;

	return -1;
}

//Signalhandler, Liste der geblockten Clients als File speichern
void signalhandler(int sig)
{
	struct host *tmp = locked;
	FILE *fp;
	char path[150];
	strcpy(path, spool);
	strcat(path, PATHLOCK);
	
	fp = fopen(path, "w");
	while(tmp != NULL)
	{
		fprintf(fp, "%s %ld\n", tmp->ip, tmp->time);
		tmp = tmp->next;
	}	
	fclose(fp);
	exit(0);
}

//Client-Handle
void * runclient(void *arg)
{

	args *arg2 = arg; //Argumente casten

	int new_socket = arg2->socket; //Socket
	struct sockaddr_in client = arg2->address; //IP und Port

	short attempt = 0; //Login-Versuche
	char buffer[BUF];
	char *username = NULL; //Benutzername	
	 int quit = 0;
	 int size = 0;


	     //Client mit Server verbunden, Willkommennachricht senden
	     if (new_socket > 0)
	     {
	        printf ("Client connected from %s:%d...\n", inet_ntoa (client.sin_addr),ntohs(client.sin_port));
	        strcpy(buffer,"Welcome to our Mail-Server, Please enter your command:\n");
	        send(new_socket, buffer, strlen(buffer),0);
	     }
	     do
	     {

	        size = readline(new_socket, buffer, BUF-1);

	        if( size > 0)
	        {
	           buffer[size] = '\0';

	           switch(findcom(buffer)) //Auswahl welcher Befehl eingegeben wurde
	           {
		           case 0: //QUIT
		           		free(username);
		           		quit = 1;
		           		break;

		           case 1: //SEND
		           		if(sendmail(new_socket, spool, username) == 0)
		           		    strcpy(buffer,"OK\n");
		           		else
			           		strcpy(buffer,"ERR\n");
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;

		           case 2: //LIST
		           		if(listmail(new_socket, spool, username) == 0)
		           		    strcpy(buffer,"OK\n");
		           		else
			           		strcpy(buffer,"ERR\n");
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;

		           case 3: //READ
		           		if(readmail(new_socket, spool, username) == 0)
		           		    strcpy(buffer,"OK\n");
		           		else
			           		strcpy(buffer,"ERR\n");;
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;

		           case 4: //DEL
		           		if(delmail(new_socket, spool, username) == 0)
		           		    strcpy(buffer,"OK\n");
		           		else
			           		strcpy(buffer,"ERR\n");
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;


		           case 5: //LOGIN
		           		if(loginuser(new_socket, &username) == 0)
		           		{
		           		    strcpy(buffer,"OK\n");
		           		}
		           		else
		           		{
			           		attempt++;
			           		strcpy(buffer,"ERR\n");
		           		}
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;

		           default: //Andere Eingabe -> Fehler
		           		strcpy(buffer,"Server: Undefinded command\n");
	        			send(new_socket, buffer, strlen(buffer),0);
		           		break;
	           }

		   //Client in Liste speichern
		   if(attempt >= 3)
	           {
		           struct host *tmp;

		           if(locked == NULL)
		           {
			           locked = malloc(sizeof(struct host));
			           locked->ip = strdup(inet_ntoa (client.sin_addr));
			           locked->time = time(NULL);
			           locked->next = NULL;
		           }
		           else
		           {
			           tmp = locked;
			           while(tmp->next != NULL)
			           		tmp = tmp->next;
			           tmp->next = malloc(sizeof(struct host));
			           tmp = tmp->next;
			           tmp->ip = strdup(inet_ntoa (client.sin_addr));
			           tmp->time = time(NULL);
			           tmp->next = NULL;
		           }

		           //Nachricht an Server senden
		           strcpy(buffer, "lock\n");
		           send(new_socket, buffer, strlen(buffer),0);	
			   printf("Client locked\n");		   
		           quit = 1;
	           }

	        }
	        else if (size == 0)
	        {
	           printf("Client closed remote socket\n");
	           free(username);
	           break;
	        }
	        else
	        {
	           perror("recv error");
	           free(username);
	           return EXIT_FAILURE;
	        }

	     } while (quit == 0);

	     close (new_socket);
}

int main (int argc, char *argv[])
{
	//Thread für Clients
	pthread_t client;	

	//Sockets for the connections
	int create_socket, new_socket;
  	socklen_t addrlen;
  	struct sockaddr_in address, cliaddress;
	
	//Variablen für Liste der geblockten Clients
	char pathlocked[150];
	FILE *fp = NULL;	

  	//Argumente überprüfen
  	if(argc != 3) //Fehler, falls ein Argument fehlt
  	{
	  	fprintf(stderr, "Usage: %s [port] [mailspool]\n", argv[0]);
	  	return -1;
  	}

	//Port speichern
  	port = strtol(argv[1], NULL, 10);

	//Mail-Spool-Dir speichern
  	spool = argv[2];

  	if(port == 0) //Fehler, falls falscher Port
  	{
	  	fprintf(stderr, "Port falsch!\n");
	  	return -1;
  	}

	//Falls Mail-Spool-Dir nicht existiert -> Programmende
  	if(access(spool, 00) == -1)
	{
		fprintf(stderr, "Mailspoolverzeichnis %s existiert nicht!\n", spool);
		return -1;
	}
	
	//Pfad geblockte Clients
	strcpy(pathlocked, spool);
	strcat(pathlocked, PATHLOCK);
	
	//geblockte Clients aus Datei auslesen
	if((fp = fopen(pathlocked, "r")) != NULL)
	{
		time_t time;
		char buf[BUF];
		char ip[BUF];
		struct host *tmp;

		while(fgets(buf, BUF-1, fp))
		{			
			sscanf(buf, "%s %ld\n", ip, &time);
			printf("%s %ld\n", ip, time);
		           if(locked == NULL)
		           {
				
			           locked = malloc(sizeof(struct host));
				   locked->ip = strdup(ip);
				   locked->time = time;			           
			           locked->next = NULL;
		           }
		           else
		           {
			           tmp = locked;
			           while(tmp->next != NULL)
			           		tmp = tmp->next;
			           tmp->next = malloc(sizeof(struct host));
			           tmp = tmp->next;
			           tmp->ip = strdup(ip);
				   tmp->time = time;	
			           tmp->next = NULL;
		           }
		}
		fclose(fp);

	}
	else
	{
		fprintf(stderr, "%s doen't exist!\n", pathlocked);
	}	
	

	//Socket erstellen
  	create_socket = socket (AF_INET, SOCK_STREAM, 0);

  	memset(&address,0,sizeof(address));
  	address.sin_family = AF_INET;
 	address.sin_addr.s_addr = INADDR_ANY;
  	address.sin_port = htons (port);

	//Socket binden
  	if (bind (create_socket, (struct sockaddr *) &address, sizeof (address)) != 0)
  	{
     	perror("bind error");
     	return EXIT_FAILURE;
  	}

	//Am Socket horchen
  	listen (create_socket, 5);

  	addrlen = sizeof (struct sockaddr_in);
	
	//Signalhandler registrieren
	signal(SIGINT,signalhandler);

	while (1)
	{
	     printf("Waiting for connections...\n");

	     args arg;
	     short lock = 0;
	     double diff;

	     //Create new client socket
	     new_socket = accept (create_socket, (struct sockaddr *) &cliaddress, &addrlen );

	     struct host *tmp = locked;
	     struct host *before = NULL;

	     //Liste der gesperrten Client durchsuchen
	     while(tmp != NULL)
	     {
		     if(strcmp(inet_ntoa(cliaddress.sin_addr), tmp->ip) == 0) //IP vergleichen, wenn gleich keine Verbindung möglich
		     {
			 lock = 1;
			 diff = difftime(time(NULL),tmp->time);

			 //Wenn gesperrte Zeit abgelaufen, Client aus der Liste löschen
			 if(diff >= IPLOCK)
			 {
				lock = 0;
				if(tmp == locked)
				{
					locked = tmp->next;
					free(tmp);
				}
				else
				{
					before->next = tmp->next;
					free(tmp);
				}
  			 }
			 break;
       		     }
                     before = tmp;
		     tmp = tmp->next;
	     }

	     //Falls Client nicht gesperrt, neuen Thread erstellen und Client Routine ausführen
	     if(lock == 0) 
	     {
	     	arg.address = cliaddress;
	     	arg.socket = new_socket;
	     	pthread_create (&client,NULL,runclient,&arg);
	     }
	     else //Client gesperrt --> Verbindung beenden zum Client
	     {			
		send(new_socket, "locked\n", 7, 0);
		close(new_socket);
		diff = difftime(time(NULL), tmp->time);			

		printf("Client %s locked! %.0lf sec left\n", inet_ntoa(cliaddress.sin_addr), IPLOCK - diff);
	     }
	}

	close (create_socket);
  return EXIT_SUCCESS;
}



