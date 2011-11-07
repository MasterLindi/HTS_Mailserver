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


//Nachricht vom Server empfangen
void receive(int mysocket, char buffer[BUF])
{
	int  size =0;

	do                                                                //Warten auf Server; Ausgabe der Antwort
    {
        size = readline(mysocket, buffer, BUF-1);

        if(size > 0)
        {
              buffer[size] = '\0';

              if(strncmp(buffer, "lock", 4) == 0)
			  {
				  close(mysocket);
				  exit(0);
			  }

              fputs(buffer,stdout);

        }
    }
    while (strncmp(buffer,"OK",2) !=  0 && strncmp(buffer,"ERR",3));
}

//Client beenden
void quitcom(int mysocket, char buffer[BUF])
{
	send(mysocket, buffer, strlen (buffer), 0);   //Client wird beendet; Verbindung wird getrennt
}

//Senden einer Email
void sendcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0;
    char *allempf, *sendempf = NULL;

    //Prüft ob der User eingeloggt ist
    size = readline(mysocket, buffer, BUF-1);

        if(size > 0)
        {
            buffer[size] = '\0';
            if(strncmp(buffer, "ERR", 3) == 0)
            {
				fputs(buffer,stdout);
				return ;
			}

        }

        eingabe = 1;
        int i = 0;
        int j;
        printf ("Geben Sie einen oder mehrere Empfänger ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        allempf = strtok(buffer, ";");



            if (allempf[strlen(allempf)-1] == '\n')
            {
                 send(mysocket, allempf, strlen(allempf), 0);
            }
            else
            {
                sendempf = (char*)malloc(strlen(allempf +1));
                strcpy(sendempf,allempf);
                strcat(sendempf,"\n");
                send(mysocket, sendempf, strlen(sendempf), 0);
            }

		size = readline(mysocket, buffer, BUF-1);
		if(size > 0)
		{
		    buffer[size] = '\0';
		    if(strncmp(buffer, "ERR", 3) == 0)
		    {
					fputs(buffer,stdout);
					return -1;
		    }
				
		}
        while (allempf != NULL)
        {

            allempf = strtok(NULL, ";");  //Mehrere Empfänger werden durch ; getrennt
            if (allempf == NULL) break;
            i++;

            if (allempf[strlen(allempf)-1] == '\n')

            {
                 send(mysocket, allempf, strlen(allempf), 0);
            }
            else
            {
                sendempf = (char*)malloc(strlen(allempf +1));
                strcpy(sendempf,allempf);
                strcat(sendempf,"\n");
                send(mysocket, sendempf, strlen(sendempf), 0);
            }
		
  	   size = readline(mysocket, buffer, BUF-1);          
    
		if(size > 0)
		{
		    buffer[size] = '\0';
		    if(strncmp(buffer, "ERR", 3) == 0)
		    {
					fputs(buffer,stdout);
					return -1;
		    }
				
		}

        }
    strcpy(buffer,"empf@aus\n");            //Info an den Server, dass keine Empfänger mehr kommen
    send(mysocket, buffer, strlen (buffer), 0);
    do      //Betreffeingabe
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

    char attach,filename[255];
    eingabe = 0;
    long sizeoffile=0;
    struct stat attribut;
    do      //Attachmentauswahl
    {    printf ("Wollen Sie eine Attachment mitsenden: (y,n) ");
         attach = getchar();
        while (getchar() != '\n');;
        if (attach == 'y')  //Ein Attachment wird mitgesendet
        {   eingabe = 1;
            printf("Geben sie den Pfad + Dateinamen an: ");
            fgets(filename,255,stdin);
            filename [strlen(filename)-1] = '\0';
            if (stat(filename, &attribut) == -1)    //Attachmentgröße wird bestimmt
            {
                fprintf(stderr,"Dateifehler\n");		
		send(mysocket, "errattach\n", 10, 0);		
		receive(mysocket, buffer);
		return -1;
            }
            else
            {
                sizeoffile= attribut.st_size;
		if(sizeoffile == 0)
		{
			fprintf(stderr,"Datei ist leer!\n");
			send(mysocket, "errattach\n", 10, 0);		
			receive(mysocket, buffer);
			return -1;
		}

            }

            sprintf( buffer, "%ld", sizeoffile );

            strcat(buffer,"\n");

            send(mysocket, buffer, strlen(buffer), 0);

            FILE *datei;

            datei = fopen(filename, "r"); //Attachment wird geöffnet

            if(NULL == datei)
            {
                printf("Konnte Datei %s nicht öffnen!\n", filename);		
                return -1;
            }
	   
	    while(fgets(buffer, BUF, datei))
	    {
	        send(mysocket, buffer, strlen (buffer), 0); //Attachment wird an den Server gesendet
	    }		
        }
        else if (attach == 'n') //Es wird kein Attachment geschickt
        {   eingabe = 1;
            strcpy(buffer,"noattach\n");
            send (mysocket, buffer,strlen(buffer),0);
        }
        else
        fprintf (stderr, "Ungültige Eingabe!\n");
    } while (eingabe != 1);


    printf("Geben Sie ihre Nachricht ein!\n");
    //Nachrichteneingabe
    do
    {
        fgets(buffer, BUF, stdin);
        send(mysocket, buffer, strlen (buffer), 0);         //Nachricht wird übermittelt
    }
    while (strncmp(buffer, ".", 1) != 0);                   //Nachricht ist so lange bis ein newline und ein Punkt kommt


    receive(mysocket, buffer);

}


void listcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0); //Auflisten aller Nachrichten + ev. Attachments
    receive(mysocket, buffer);

}

void readcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0,totallength=0,templength = 0;
    char attach;

    size = readline(mysocket, buffer, BUF-1);           //Warten auf Antwort von Server

        if(size > 0)
        {
            buffer[size] = '\0';
            if(strncmp(buffer, "ERR", 3) == 0)
            {
                fputs(buffer,stdout);
                return ;
            }

        }

    //Nachrichtennummer
    do
    {
        eingabe = 1;
        printf ("Geben Sie eine Nachrichtennummer ein: ");  //Eingabe der Nachrichtennummer
        fgets (buffer, BUF, stdin);
        int j;
        for(j = 0; j < strlen(buffer)-1; j++)
            {
                if(!isdigit(buffer[j])) //Überprüfung der Nachrichtennummer
                {
                        printf ("Üngultige Zeichen!\n");
                        eingabe = 0;
                        break;
                }
            }

    }while (eingabe !=1);

     send(mysocket, buffer, strlen (buffer), 0);

        size = readline(mysocket, buffer, BUF-1);
         if(size > 0)
        {
            buffer[size] = '\0';
            if ((strncmp (buffer,"OK",2))==0 || (strncmp(buffer,"ERR",3))== 0) //Überprüfen der Serverantwort ob Nachricht vorhanden
            {
                fputs(buffer,stdout);
                return;
            }
            if ((strncmp(buffer,"true",strlen("true")))== 0)  //Wenn es die Nachricht gibt
            {
                    eingabe = 0;
                    do
                    {     printf ("Diese Nachricht enthält ein Attachment.\nWollen Sie das Attachment anzeigen:(y,n)");
                         attach = getchar();
                        while (getchar() != '\n');;     //Auswahl ob man das Attachment sehen will
                        if (attach == 'y')
                        {   eingabe = 1;
                            printf ("\nAttachment:\n");
                            strcpy(buffer,"showattach\n");
                            send(mysocket, buffer, strlen (buffer), 0);

                            size = readline(mysocket, buffer, BUF-1);
                            if (size > 0)
                            {       buffer[size] = '\0';
                                  totallength = strtol(buffer,NULL,10);
                            }

                            do
                            {
                                    size = readline(mysocket, buffer, BUF-1);       //Antwort des Server abwarten bzw. ausgeben

                                    if(size > 0)
                                    {
                                        buffer[size] = '\0';         //Attachment wird angezeigt
                                        if (totallength == templength) break;
                                        fputs(buffer,stdout);
                                        templength = templength+strlen(buffer);
                                    }

                            }while (templength!=totallength);

                        }
                        else if (attach == 'n')  //Attachment wird nicht angezeigt
                        {   eingabe = 1;
                            printf ("Ihr Attachment wird nicht angezeigt.\n");
                            strcpy(buffer,"false\n");
                            send(mysocket, buffer, strlen (buffer), 0);
                        }
                        else
                        fprintf (stderr,"Keine gültige Eingabe!\n");
                    }while (eingabe != 1);
            }
            else    // Zu dieser Nachricht gibt es kein Attachment
            {
                printf ("Diese Nachricht enthält kein Attachment.\n");

            }

		}
		else
            return;

    printf ("\nNachricht:\n");

    receive(mysocket, buffer);

}

void delcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe, size =0;


    size = readline(mysocket, buffer, BUF-1);           //Warten auf Antwort von Server

        if(size > 0)
        {
            buffer[size] = '\0';
		
            if(strncmp(buffer, "ERR", 3) == 0)  //Überprüfen ob es die Nachricht gibt
            {
			fputs(buffer,stdout);
			return ;
		}
	  

        }
     
        printf ("Geben Sie eine Nachrichtennummer ein: ");
        fgets (buffer, BUF, stdin);        
     	
	send(mysocket, buffer, strlen (buffer), 0);
	
	receive(mysocket, buffer);
	
}

void logincom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe;
    do                                                  //Eingabe laut Protokoll
    {
        eingabe = 1;
        printf ("Geben Sie ihren LDAP-Usernamen ein [max. 8 Zeichen]: "); //Eingabe des Ldap Usernamen
        fgets (buffer, BUF, stdin);
        if (strlen(buffer) > 9)
        {
            printf("Ungültige Usernamelänge!\n");
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


        printf ("Geben Sie ihr Passwort ein: ");
        char password[50], temp;
        int passwindex = 0;
        while ((temp = getch()) != 10)       // Solange kein Enter eingegeben wird
        {
            if ( temp == 127 )
            {		passwindex--;
                    password[passwindex] = '\0';
                     printf("\b \b"); //Zeichen löschen
            } else
            {
                printf("*");    //Statt dem Passwort werden * ausgegeben
                password[passwindex] = temp;
                passwindex++;
                password[passwindex] = '\0';
            }
        }

        strcpy(buffer,password);
        strcat(buffer,"\n");

        send(mysocket, buffer, strlen (buffer), 0); //Passwort wird an den Server gesendet

        receive(mysocket, buffer);
}

int getch() //Funktion für Zeichenweises Einlesen ohne Enter

{

   static int ch = -1, fd = 0;

   struct termios neu, alt;

   fd = fileno(stdin);

   tcgetattr(fd, &alt);

   neu = alt;

   neu.c_lflag &= ~(ICANON|ECHO);

   tcsetattr(fd, TCSANOW, &neu);

   ch = getchar();

   tcsetattr(fd, TCSANOW, &alt);

   return ch;

}

