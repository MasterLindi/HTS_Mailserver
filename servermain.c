#include "serverfunctions.h"

#define BUF 1024

short port = 0;

const char *options[] = { "quit", "send", "list", "read", "del" };

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
	
	return -1;
}

int main (int argc, char *argv[])
{
	//Sockets for the connections
	int create_socket, new_socket;
	char *spool = NULL;
	
  	socklen_t addrlen;  
  		
  	//Buffer for Data
  	char buffer[BUF];  
  		
  	int size;  	
  	struct sockaddr_in address, cliaddress;
  	
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
  	
  	short quit = 0;

	 while (1) 
	 {
	     printf("Waiting for connections...\n");
	     
	     //Create new client socket
	     new_socket = accept (create_socket, (struct sockaddr *) &cliaddress, &addrlen );
	     
	     quit = 0;
	     
	     if (new_socket > 0)
	     {
	        printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
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
		           		quit = 1;
		           		break;
		           
		           case 1: //SEND
		           		if(sendmail(new_socket, spool) == 0)
		           		    strcpy(buffer,"OK\n");					
		           		else
			           		strcpy(buffer,"ERR\n");
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;
		           
		           case 2: //LIST
		           		if(listmail(new_socket, spool) == 0)
		           		    strcpy(buffer,"OK\n");					
		           		else
			           		strcpy(buffer,"ERR\n");;
		           		send(new_socket, buffer, strlen(buffer),0);          		
		           		break;
		           		
		           case 3: //READ
		           		if(readmail(new_socket, spool) == 0)
		           		    strcpy(buffer,"OK\n");					
		           		else
			           		strcpy(buffer,"ERR\n");;
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;
		           		
		           case 4: //DEL
		           		if(delmail(new_socket, spool) == 0)
		           		    strcpy(buffer,"OK\n");					
		           		else
			           		strcpy(buffer,"ERR\n");;
		           		send(new_socket, buffer, strlen(buffer),0);
		           		break;
		           	
		           default: //Andere Eingabe -> Fehler
		           		strcpy(buffer,"Undefinded command\n");
	        			send(new_socket, buffer, strlen(buffer),0);
		           		break;
	           }
	           
	           
	        }
	        else if (size == 0)
	        {
	           printf("Client closed remote socket\n");
	           break;
	        }
	        else
	        {
	           perror("recv error");
	           return EXIT_FAILURE;
	        }
	        
	     } while (quit == 0);
	     
	     close (new_socket);
	  }
	  
	  close (create_socket);
  return EXIT_SUCCESS;
}



