#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "clientfunctions.h"
#define BUF 1024

short port = 0;

const char *options[] = { "quit", "send", "list", "read", "del" };  //Command-options

int findcom(char s[BUF])            //Command-finder
{
	int i;

	for(i = 0; i < 4; i++)
	{
		if(strncmp(s, options[i], 4) == 0)      //char der LÄnge Buf wird mit dem options array verglichen
			return i;
	}

	if(strncmp(s, options[4], 3) == 0)
			return 4;

	return -1;
}



int main (int argc, char **argv) {
  int create_socket;
  char buffer[BUF];
  struct sockaddr_in address;
  int size;

  if( argc < 2 ){
     printf("Usage: %s ServerAdresse\n", argv[0]);      //Überprüfung der Eingabeparameter
     exit(EXIT_FAILURE);
  }

  if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)  //Socket wird erstellt
  {
     perror("Socket error");
     return EXIT_FAILURE;
  }
    port = strtol(argv[2], NULL, 10);

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons ((port));
  inet_aton (argv[1], &address.sin_addr);

  if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0)  //Verbindung zu Server wird hergestellt
  {
     printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
     size=recv(create_socket,buffer,BUF-1, 0);
     if (size>0)
     {
        buffer[size]= '\0';
        printf("%s",buffer);
     }
  }
  else
  {
     perror("Connect error - no server available");     //Verbindungsfehler
     return EXIT_FAILURE;
  }

  do {
     printf ("Enter command: ");        //eingegebener Command wird eingelesen
     fgets (buffer, BUF, stdin);

     switch(findcom(buffer))            //Command wird festgestellt und Funktion wird aufgerufen
	           {
		           case 0:
		           		quitcom(create_socket, buffer);

		           		break;

		           case 1:
		           		sendcom(create_socket, buffer);
                        break;

		           case 2:
		           		listcom(create_socket,buffer);
		           		break;
		           case 3:
		           		readcom(create_socket,buffer);
		           		break;

		           case 4:
		           		delcom(create_socket, buffer);
		           		break;

		           default:
		           		printf("Undefinded command\n");
		           		break;
	           }




  }
  while (strcmp (buffer, "quit\n") != 0);

  close (create_socket);

  return EXIT_SUCCESS;
}
