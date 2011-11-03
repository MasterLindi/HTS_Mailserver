#include "clientfunctions.h"

#define BUF 1024

ssize_t readline (int fd, void *vptr, size_t maxlen)
{
 	ssize_t   n, rc ;
	char c, *ptr ;
	ptr = vptr ;
	for (n = 1 ; n < maxlen ; n++)
	{
	  again:
	    if ( (rc = read(fd,&c,1)) == 1)
	    {
	      *ptr++ = c ;
	       if (c == '\n')
	         break ;                  // newline ist stored, like fgets()
	     } else if (rc == 0)
	     {
	         if (n == 1)
	           return (0) ;           // EOF, no data read
	         else
	           break ;                // EOF, some data was read
	     } else
	     {
	         if (errno == EINTR)
	           goto again ;
	         return (-1) ;            // error, errno set by read()
	     } ;
	 } ;
	 *ptr = 0 ;                       // null terminate like fgets()
	 return (n) ;
}

void quitcom(int mysocket, char buffer[BUF])
{
	send(mysocket, buffer, strlen (buffer), 0);   //Client wird beendet; Verbindung wird getrennt
}


void sendcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0;

    do                                                                                //Eingabeüberprüfung
    {
        eingabe = 1;
        printf ("Geben Sie einen Absender ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 8)
        {
            printf("Ungültige Absenderlänge!\n");
            eingabe = 0;
        }
        else
        {
	        int j;
            for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isalnum(buffer[j]))
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                }
            }
        }
    }
    while (eingabe !=1);

    send(mysocket, buffer, strlen (buffer), 0);                             //Absender wird an den Server übermittelt

    do
    {
        eingabe = 1;
        printf ("Geben Sie einen Empfänger ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 8)
        {
            printf("Ungültige Empfängerlänge!\n");
            eingabe = 0;
        }
        else
        {
	        int j;
            for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isalnum(buffer[j]))
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                }
            }
        }
    }
    while (eingabe !=1);

    send(mysocket, buffer, strlen (buffer), 0);                     //Empfänger wird übermittelt

    do
    {
        eingabe = 1;
        printf ("Geben Sie einen Betreff ein [max. 80 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 80)
        {

            printf("Ungültige Betrefflänge!\n");
            eingabe = 0;
        }
        else
        {
	        int j;
              for(j = 0; j < strlen(buffer)-1; j++)
                {
                    if(!isalnum(buffer[j])&& buffer[j] != ' ')
                    {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                    }
                }

        }

    }
    while (eingabe !=1);

    send(mysocket, buffer, strlen (buffer), 0);                             //Betreff wird übermittelt

    printf("Geben Sie ihre Nachricht ein!\n");

    do
    {
        fgets(buffer, BUF, stdin);
        send(mysocket, buffer, strlen (buffer), 0);         //Nachricht wird übermittelt
    }
    while (strncmp(buffer, ".", 1) != 0);                   //Nachricht ist so lange bis ein newline und ein Punkt kommt

    do
    {

        size = readline(mysocket, buffer, BUF-1);           //Warten auf Antwort von Server

        if(size > 0)
        {
            buffer[size] = '\0';
            fputs(buffer,stdout);
        }

    }
    while (size == 0);

}


void listcom(int mysocket, char buffer[BUF])
{
 	send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0;
    do                                                              //Eingabeüberprüfung
    {
        eingabe = 1;
        printf ("Geben Sie ihren Usernamen ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 8)
        {
            printf("Ungültige Userlänge!\n");
            eingabe = 0;
        }
        else
        {
	        int j;
            for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isalnum(buffer[j]))
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                }
            }
        }
    }while (eingabe !=1);
     send(mysocket, buffer, strlen (buffer), 0);

      do                                                                //Warten auf Server; Ausgabe der Antwort
    	{
        size = readline(mysocket, buffer, BUF-1);

        if(size > 0)
        {
                buffer[size] = '\0';
                fputs(buffer,stdout);
        }
    }
    while (strncmp(buffer,"OK",2) !=  0 && strncmp(buffer,"ERR",3));
}

void readcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0;
    do                                                                              //Eingabe laut Protokoll
    {
        eingabe = 1;
        printf ("Geben Sie ihren Usernamen ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 8)
        {
            printf("Ungültige Userlänge!");
            eingabe = 0;
        }
        else
        {
	        int j;
            for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isalnum(buffer[j]))
                {
                        printf("Üngultige Zeichen!\n");
                        eingabe = 0;
                        break;
                }
            }
        }
    }while (eingabe !=1);
     send(mysocket, buffer, strlen (buffer), 0);


    do
    {
        eingabe = 1;
        printf ("Geben Sie eine Nachrichtennummer ein: ");
        fgets (buffer, BUF, stdin);
        int j;
        for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isdigit(buffer[j]))
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                        break;
                }
            }

    }while (eingabe !=1);
     send(mysocket, buffer, strlen (buffer), 0);


   do
	{
		size = readline(mysocket, buffer, BUF-1);       //Antwort des Server abwarten bzw. ausgeben

		if(size > 0)
		{
			buffer[size] = '\0';

			fputs(buffer,stdout);

		}

	}
	while(strncmp(buffer,"OK",2) !=  0 && strncmp(buffer,"ERR",3));
}

void delcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0;
    do                                                  //Eingabe laut Protokoll
    {
        eingabe = 1;
        printf ("Geben Sie ihren Usernamen ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 8)
        {
            printf("Ungültige Userlänge!\n");
            eingabe = 0;
        }
        else
        {
	        int j;
            for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isalnum(buffer[j]))
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                }
            }
        }
    }while (eingabe !=1);
     send(mysocket, buffer, strlen (buffer), 0);

     do
    {
        eingabe = 1;
        printf ("Geben Sie eine Nachrichtennummer ein: ");
        fgets (buffer, BUF, stdin);
        int j;
        for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isalnum(buffer[j]))
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                }
            }

    }while (eingabe !=1);
     send(mysocket, buffer, strlen (buffer), 0);

      do                                                    //Ausgabe der Serverantwort
    {

        size = readline(mysocket, buffer, BUF-1);

        if(size > 0)
        {
                buffer[size] = '\0';
                fputs(buffer,stdout);
        }

    }   while (size == 0);
}
