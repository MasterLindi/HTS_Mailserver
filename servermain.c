#include "serverfunctions.h"

#define BUF 1024
#define IPLOCK 30

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

void * runclient(void *arg)
{
	args *arg2 = arg;
	
	
	int new_socket = arg2->socket;
	struct sockaddr_in client = arg2->address;
	short attempt = 0;
	short login = 0;
	char buffer[BUF];  
	char *username = NULL;
	 int quit = 0;
	 int size = 0;
	     
	     if (new_socket > 0)
	     {
	        //printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
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
			           		strcpy(buffer,"ERR\n");;
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
			           		strcpy(buffer,"ERR\n");;
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
		           		strcpy(buffer,"Undefinded command\n");
	        			send(new_socket, buffer, strlen(buffer),0);
		           		break;
	           }
	           
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
			            locked->ip = strdup(inet_ntoa (client.sin_addr));
			           tmp->time = time(NULL);
			           tmp->next = NULL;
		           }
		           
		           strcpy(buffer, "Login three times failed! You ip-address is locked!\n");
		           send(new_socket, buffer, strlen(buffer),0);
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
	pthread_t client;
	
	
	
	//Sockets for the connections
	int create_socket, new_socket;
  	socklen_t addrlen;  
  	struct sockaddr_in address, cliaddress;
  	
  	//Argumente überprüfen
  	if(argc != 3) //Fehler, falls ein Argument fehlt
  	{
	  	fprintf(stderr, "Usage: %s [port] [mailspool]\n", argv[0]);
	  	return -1;
  	}
  	
  	port = strtol(argv[1], NULL, 10);
  	spool = argv[2];
  	
  	if(port == 0) //Fehler, falls falscher Port
  	{
	  	fprintf(stderr, "Port falsch!\n");
	  	return -1;
  	}
  	
  	if(access(spool, 00) == -1)	//Existiert das Verzeichnis nicht, dann Fehler
	{
		fprintf(stderr, "Mailspoolverzeichnis %s existiert nicht!\n", spool);
		return -1;
	}
	
	//Socket erstellen
  	create_socket = socket (AF_INET, SOCK_STREAM, 0);

  	memset(&address,0,sizeof(address));
  	address.sin_family = AF_INET;
 	address.sin_addr.s_addr = INADDR_ANY;
  	address.sin_port = htons (port);

  	if (bind (create_socket, (struct sockaddr *) &address, sizeof (address)) != 0) 
  	{
     	perror("bind error");
     	return EXIT_FAILURE;
  	}
  	
  	listen (create_socket, 5);
  
  	addrlen = sizeof (struct sockaddr_in);
  	
  	

	while (1) 
	{
	     printf("Waiting for connections...\n");
	     
	     args arg;      
	     short lock = 0;
	     struct host *tmp = locked;
	     
	     //Create new client socket
	     new_socket = accept (create_socket, (struct sockaddr *) &cliaddress, &addrlen );	 
	     
	     while(tmp != NULL)
	     {
		     printf("%s %s\n", inet_ntoa(cliaddress.sin_addr), tmp->ip);
		     if(strcmp(inet_ntoa(cliaddress.sin_addr), tmp->ip) == 0)
		     {
		     	lock = 1;
		     	break;
		     }
		     
		     tmp = tmp->next;
	     } 
	     
	     if(lock == 0)
	     {	     
	     	arg.address = cliaddress;
	     	arg.socket = new_socket;
	     	pthread_create (&client, NULL, runclient, &arg);	
	     }   
	     else
	     {
		     printf("Client locked");
	     }
	}
	  
	close (create_socket);
  return EXIT_SUCCESS;
}



