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
				return -1;
			}
				
        }

        eingabe = 1;
        int i = 0;
        int j;
        printf ("Geben Sie einen oder mehrere Empfänger ein [max. 8 Zeichen]: ");
        fgets (buffer, BUF, stdin);
        allempf = strtok(buffer, ";");
        if (strlen (allempf) >9)
        {
            fprintf (stderr,"1.te Empfängerlänge zu lang!\n");
                        return;
        }
        else
            {
				     for(j = 0; j < strlen(allempf)-1; j++)
                    {
                        if(!isalnum(allempf[j]))
                        {
                                fprintf (stderr, "\nÜngultige Zeichen beim 1.ten Empfänger\n");
                                return;

                        }
                    }
            }


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
        while (allempf != NULL)
        {

            allempf = strtok(NULL, ";");
            if (allempf == NULL) break;
            i++;
            if(strlen(allempf) > 9)
            {
                fprintf (stderr, "%d.te Empfängerlänge zu lang!\n",i+1);
                return;
            }
            else
            {

                    for(j = 0; j < strlen(allempf)-1; j++)
                    {
                        if(!isalnum(allempf[j]))
                        {
                                fprintf (stderr, "\nÜngultige Zeichen beim %d.ten Empfänger\n",i+1);
                                return;

                        }
                    }
            }
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

        }
    strcpy(buffer,"empfaus\n");
    send(mysocket, buffer, strlen (buffer), 0);
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

    char attach,filename[255];
    eingabe = 0;
    do
    {    printf ("Wollen Sie eine Attachment mitsenden: (y,n) ");
         attach = getchar();
        while (getchar() != '\n');;
        if (attach == 'y')
        {   
	    eingabe = 1;
            printf("Geben sie den Pfad + Dateinamen an: ");
            fgets(filename,255,stdin);
            filename [strlen(filename)-1] = '\0';
            FILE *datei;

            datei = fopen(filename, "r");

            if(NULL == datei)
            {
                fprintf(stderr,"Konnte Datei %s nicht öffnen!\n", filename);
                return;
            }

           while(fgets(buffer, BUF, datei))
            {

                send(mysocket, buffer, strlen (buffer), 0);
            }
        	strcpy(buffer,"attachaus\n");
        	send(mysocket, buffer, strlen (buffer), 0);
        }
        else if (attach == 'n')
        {   eingabe = 1;
            strcpy(buffer,"noattach\n");
            send (mysocket, buffer,strlen(buffer),0);
        }
        else
        	fprintf (stderr, "Ungültige Eingabe!\n");
    } while (eingabe != 1);


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
    
    receive(mysocket, buffer);
}

void readcom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0); 
    
    int eingabe, size =0;
    char attach;	
    
    size = readline(mysocket, buffer, BUF-1);           //Warten auf Antwort von Server

        if(size > 0)
        {
            buffer[size] = '\0';
            if(strncmp(buffer, "ERR", 3) == 0)
            {
				fputs(buffer,stdout);
				return -1;
			}
				
        }
	
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

        size = readline(mysocket, buffer, BUF-1);
         if(size > 0)
		{
			buffer[size] = '\0';
            if ((strncmp (buffer,"OK",2))==0 || (strncmp(buffer,"ERR",3))== 0)
            {
                fputs(buffer,stdout);
                return;
            }
			if ((strncmp(buffer,"true",strlen("true")))== 0)
			{
                    eingabe = 0;
                    do
                    {     printf ("Diese Nachricht enthält ein Attachment.\nWollen Sie das Attachment anzeigen:(y,n)");
                         attach = getchar();
                        while (getchar() != '\n');;
                        if (attach == 'y')
                        {   eingabe = 1;
                            printf ("\nAttachment:\n");
                            strcpy(buffer,"showattach\n");
                            send(mysocket, buffer, strlen (buffer), 0);
                            do
                            {
                                    size = readline(mysocket, buffer, BUF-1);       //Antwort des Server abwarten bzw. ausgeben

                                    if(size > 0)
                                    {
                                        buffer[size] = '\0';
                                        if ((strncmp(buffer,"attachaus",strlen("attachaus")))== 0) break;
                                        fputs(buffer,stdout);
                                    }

                            }while ((strncmp(buffer,"attachaus",strlen("attachaus")))!= 0);

                        }
                        else if (attach == 'n')
                        {   eingabe = 1;
                            printf ("Ihr Attachment wird nicht angezeigt.\n");
                            strcpy(buffer,"false\n");
                            send(mysocket, buffer, strlen (buffer), 0);
                        }
                        else
                        fprintf (stderr,"Keine gültige Eingabe!\n");
                    }while (eingabe != 1);
            }
            else
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
            if(strncmp(buffer, "ERR", 3) == 0)
            {
				fputs(buffer,stdout);
				return -1;
			}
				
        }

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

    receive(mysocket, buffer);
}

void logincom(int mysocket, char buffer[BUF])
{
    send(mysocket, buffer, strlen (buffer), 0);

    int eingabe,size =0;
    do                                                  //Eingabe laut Protokoll
    {
        eingabe = 1;
        printf ("Geben Sie ihren LDAP-Usernamen ein [max. 8 Zeichen]: ");
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
                printf("*");
                password[passwindex] = temp;
                passwindex++;
                password[passwindex] = '\0';
            }
        }

        strcpy(buffer,password);
        strcat(buffer,"\n");
    
        send(mysocket, buffer, strlen (buffer), 0);

       receive(mysocket, buffer);
}

int getch()

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

