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
    char *allempf, *sendempf = NULL;

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

    printf("Geben Sie ihre Nachricht ein!\n");

    do
    {
        fgets(buffer, BUF, stdin);
        send(mysocket, buffer, strlen (buffer), 0);         //Nachricht wird übermittelt
    }
    while (strncmp(buffer, ".", 1) != 0);                   //Nachricht ist so lange bis ein newline und ein Punkt kommt

    printf ("Wollen Sie eine Attachment mitsenden: (y,n) ");
    char atach,filename[255];
    atach = getchar();
    while (getchar() != '\n');;
    if (atach == 'y')
    {
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
        char zeichen;

       while(fgets(buffer, BUF, datei))
        {


        }

    }
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

    int  size =0;
   /* do                                                              //Eingabeüberprüfung
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
     send(mysocket, buffer, strlen (buffer), 0);*/

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
   /* do                                                                              //Eingabe laut Protokoll
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
    */

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
    /*do                                                  //Eingabe laut Protokoll
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
     send(mysocket, buffer, strlen (buffer), 0);*/

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

     do                                                  //Eingabe laut Protokoll
    {
        eingabe = 1;
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
        printf("Ausgabe: %s %d\n", buffer, eingabe);


    }while (eingabe !=1);
    
    printf("send_before");
     send(mysocket, buffer, strlen (buffer), 0);
     printf("send_after");

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
/*char* readpassw ()
{   char password[15], temp;
	int passwindex = 0;
	while ((temp = getch()) != 13)
    {	if (passwindex ==15-1)break;
		if ( temp == 8 )
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

	return password;

}*/
