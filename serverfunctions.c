#include "serverfunctions.h"

#define BUF 1024
#define OVERFLOW 1

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

int loginuser(int socket ,char **username)
{
	int size;
	int i;
	int j;
	int len;
	char buffer[BUF];
	char *user;
	char *pw;

	/*if(username != NULL)
	return -1;*/

	for(i = 0; i < 2; i++)
	{
		size = readline(socket, buffer, BUF-1);

		if(size > 0)
		{
			buffer[size] = '\0';

			switch(i)
			{
				case 0: //Benutzer
					user = (char *)malloc(strlen(buffer)+1);
					strcpy(user, buffer);

					len = strlen(user);

					if(len-OVERFLOW > 8) //Länge des Sender größer 8, wenn ja Fehler
						return -1;

					//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++)
					{
						if(!isalnum(user[j]))
						{
							free(user);
							return -1;
						}
					}
					break;

				case 1: //Passwort
					pw = (char *)malloc(strlen(buffer)+1);
					strcpy(pw, buffer);
					pw[strlen(pw)-1] = '\0';
					break;
			}
		}
		else
		{
			return -1;
		}

	}

	 *username = (char *)malloc(strlen(user)+1);
	strcpy(*username, user);

	free(user);
	free(pw);

	return 0;
}


//Mail senden
int sendmail(int socket, char *spool, char *username)
{
	int size;
	int i = 0;
	int j;
	int len = 0;
	int quit = 0;
	int id = 0;
	int anzempf = 0;
	int attach = 1;

	char bufid[33];
	FILE *fp = NULL;
	char buffer[BUF];
	char mailpath[150];
	char confpath[150];
	char attachpath[150];
	char **receiver = NULL;
	char *subject = NULL;
	char *content = NULL;
	char *attachment = NULL;

    receiver = malloc(100*sizeof(char *));

	if(username == NULL)
		return -1;

	do
	{
		size = readline(socket, buffer, BUF-1);

		if(size > 0)
		{
			buffer[size] = '\0';

			switch(i)
			{
				case 0: //Empfänger
                    if ((strncmp(buffer,"empfaus",strlen("empfaus")))== 0) break;
                    else
                    {
                        receiver[anzempf]= (char *)malloc(strlen(buffer)+1);
                        strcpy(receiver[anzempf], buffer);
                        len = strlen(receiver[anzempf]);

                        if(len-OVERFLOW > 8) //Länge des Empfängers größer 8, wenn ja Fehler
                        return -1;

                        //Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
                        for(j = 0; j < len-OVERFLOW; j++)
                        {
                            if(!isalnum(receiver[anzempf][j]))
                            {
                                free(receiver);
                                return -1;
                            }
                        }
                        anzempf++;
                        break;
                    }

				case 1: //Betreff
					subject = (char *)malloc(strlen(buffer)+1);
					strcpy(subject, buffer);

					len = strlen(subject);

					if(len-OVERFLOW > 80) //Länge des Betreffs größer 80, wenn ja Fehler
						return -1;

					//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++)
					{
						if(!isalnum(subject[j]) && subject[j] != ' ')
						{
							free(receiver);
							free(subject);
							return -1;
						}
					}

					break;

                case 2: //Attachment
                    if ((strncmp(buffer, "noattach", strlen("noattach")))==0)
                    {
                        attach = 0;
                        i++;
                        break;
                    }
                    if((strncmp(buffer, "attachaus", strlen("attachaus"))) == 0)  //Ende des Attachment
					{
						break;
					}
					else //Einlesen des Inhaltes
					{
						if(attachment == NULL)
						{
							attachment = (char *)malloc(strlen(buffer)+1);

							if(attachment == NULL)
							{
								free(receiver);
								free(subject);
								free(content);
								free (attachment);
								return -1;
							}

							strcpy(attachment, buffer);
						}
						else
						{
							len = strlen(attachment);
							attachment = (char *)realloc(attachment, strlen(buffer)+len+1);

							if(attachment == NULL)
							{
								free(receiver);
								free(subject);
								free(content);
								free(attachment);
								return -1;
							}

							strcat(attachment, buffer);
						}
					}
					break;

				default: //Inhalt

					if(strncmp(buffer, ".", 1) == 0) //Ende der Nachricht
					{
						if(content == NULL)
						{
							content = (char *)malloc(2);
							strcpy(content, " ");
						}
						quit = 1;
					}
					else //Einlesen des Inhaltes
					{
						if(content == NULL)
						{
							content = (char *)malloc(strlen(buffer)+1);

							if(content == NULL)
							{
								free(receiver);
								free(subject);
								free(content);
								return -1;
							}

							strcpy(content, buffer);
						}
						else
						{
							len = strlen(content);
							content = (char *)realloc(content, strlen(buffer)+len+1);

							if(content == NULL)
							{
								free(receiver);
								free(subject);
								free(content);
								return -1;
							}

							strcat(content, buffer);
						}
					}
					break;
			}

		}
		else
		{
			return -1;
		}

		if ((strncmp(buffer,"empfaus",strlen("empfaus")))==0 || i > 0)
		{   if (i ==2 && (strncmp(buffer, "attachaus", strlen("attachaus"))==0)) i++;
            if (i!= 2) i++;
		}


	}
	while(quit == 0);

    for (int i = 0; i<anzempf; i++)
    {
        //Verzeichnispfad erstellen

	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, receiver[i], strlen(receiver[i])-OVERFLOW);
	strcpy(confpath, mailpath);
	strcat(confpath, "/conf.ini");
	strcpy(attachpath,mailpath);



		//Existiert das Verzeichnis bereits
		if(access(mailpath, 00) == -1)
		{
			mkdir(mailpath, 0777); //Verzeichnis für den Benutzer erstellen
			fp = fopen(confpath, "w");
			fprintf(fp, "%d", 1);
			fclose(fp);
		}


		//ID aus dem Config-File lesen
		fp = fopen(confpath, "r");
		fscanf(fp, "%s", bufid);
		fclose(fp);




	//Pfad fertig bauen
	strcat(mailpath, "/");
	strcat(mailpath, bufid);

     strcat (attachpath, "/");
    strcat (attachpath, bufid);
    strcat(attachpath,".attach");


	//Email abspeichern
	if((fp = fopen(mailpath, "w")) == NULL)
		return -1;

	fputs(username, fp);
	fputs(receiver[i], fp);
	fputs(subject, fp);
	fputs(content, fp);

	fclose(fp);

		//Email abspeichern
		if((fp = fopen(mailpath, "w")) == NULL)
			return -1;


		fputs(username, fp);
		fputs(receiver[i], fp);
		fputs(subject, fp);
		fputs(content, fp);

		fclose(fp);

		//ID im Config-File erhöhen und in Datei schrieben
		id = strtol(bufid, NULL, 10);
		id ++;

        if (attach != 0)
        {
            if((fp = fopen(attachpath, "w")) == NULL) return -1;
            fputs(attachment,fp);
            fclose(fp);
        }


		fp = fopen(confpath, "w");
		fprintf(fp, "%d", id);
		fclose(fp);


    }
	//Speicher freigeben
	free(receiver);
	free(subject);
	free(content);
	free(attachment);

	return 0;
}

//Emails eines Benutzers anzeigen
int listmail(int socket, char *spool, char *username)
{
	int i;
	char buffer[BUF];
	char subject[81];
	char mailpath[150];
	char msgpath[150];
	FILE *fp = NULL;
	DIR *ver;
	struct dirent *p;

	if(username == NULL)
		return -1;


	//Pfad zum Benutzer erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, username, strlen(username)-OVERFLOW);

	//Benutzer nicht vorhanden
	if(access(mailpath, 00) == -1)
		{
			return -1;
		}

		if((ver = opendir(mailpath)) != NULL) //Verzeichnis öffnen
		{
			while((p = readdir(ver)) != NULL) //Ausgabe jeder Datei aus dem Verzeichnis
			{
				if( strcmp((*p).d_name, "..") != 0 &&
					strcmp((*p).d_name, ".") != 0 &&
					strcmp((*p).d_name, "conf.ini") != 0
				)
				{
					//Betreff jeder Nachricht auslesen
					strcpy(msgpath, mailpath);
					strcat(msgpath, "/");
					strcat(msgpath, (*p).d_name);

					if((fp = fopen(msgpath, "r")) == NULL)
						return -1;

					for(i = 0; i < 2; i++)
						fgets(subject, 81, fp);

					fgets(subject, 81, fp);

					fclose(fp);

					strcpy(buffer,(*p).d_name);
					strcat(buffer, "  ");
					strcat(buffer, subject);

	        		send(socket, buffer, strlen(buffer),0);
				}
			}
		}

		closedir(ver);

	return 0;
}

//Email auslesen
int readmail(int socket, char *spool, char *username)
{
	int size;
	int i;
	int j;
	int len;
	FILE *fp, *fpattach;
	char buffer[BUF];
	char mailpath[150];
	char attachpath[150];
	char *msg = NULL;

	if(username == NULL)
		return -1;

		size = readline(socket, buffer, BUF-1);

		if(size > 0)
		{
			buffer[size] = '\0';

			msg = (char *)malloc(strlen(buffer)+1);
			strcpy(msg, buffer);

			len = strlen(msg);

			//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
			for(j = 0; j < len-OVERFLOW; j++)
			{
				if(!isdigit(msg[j]))
				{
					free(msg);
					return -1;
				}
			}
		}
		else
		{
			return -1;
		}

	//Pfad erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, username, strlen(username)-OVERFLOW);

	//Benutzer nicht vorhanden
	if(access(mailpath, 00) == -1)
	{
		return -1;
	}

	strcat(mailpath, "/");
	strncat(mailpath, msg, strlen(msg)-OVERFLOW);
	strcpy(attachpath,mailpath);
	strcat(attachpath,".attach");

	//Mail-File öffnen, Prüfung ob Mail vorhanden
	if((fp = fopen(mailpath, "r")) == NULL)
	{
		return -1;
	}

	//Prüfen ob Nachricht ein Attachment enthält
	if ((fpattach = fopen(attachpath,"r"))==NULL)
	{   strcpy (buffer,"false\n");
	    send(socket, buffer, strlen(buffer),0);
	}
	else
	{
	    strcpy (buffer,"true\n");
	    send(socket, buffer, strlen(buffer),0);

	    //Antwort ob Benutzer Mail lesen will
        size = readline(socket, buffer, BUF-1);
        if (size > 0)
        {
            buffer[size] = '\0';
            if ((strncmp (buffer,"showattach",strlen("showattach")))== 0)
            {
                while (fgets(buffer,BUF,fpattach))
                {
                    send(socket,buffer,strlen(buffer),0);
                }
                strcpy(buffer,"attachaus\n");
                send(socket,buffer,strlen(buffer),0);
            }
        }
        fclose(fpattach);

	}



	//Gewählte Mail an Client schicken
	while(fgets(buffer, BUF, fp))
	{
	    send(socket, buffer, strlen(buffer),0);
	}

	//Mail-File schließen
	fclose(fp);


	//Speicher freigeben
	free(msg);

	return 0;
}

int delmail(int socket, char *spool, char *username)
{
	int size;
	int j;
	int len;
	FILE *fp;
	char buffer[BUF];
	char mailpath[150];
	char attachpath[150];
	char *msg = NULL;

	if(username == NULL)
		return -1;

		size = readline(socket, buffer, BUF-1);

		if(size > 0)
		{
			buffer[size] = '\0';

			msg = (char *)malloc(strlen(buffer)+1);
			strcpy(msg, buffer);

			len = strlen(msg);

			//Prüfung ob alle Zeichen numerisch sind, wenn nicht, Fehler
			for(j = 0; j < len-OVERFLOW; j++)
			{
				if(!isdigit(msg[j]))
				{
					free(msg);
					return -1;
				}
			}
		}
		else
		{
			return -1;
		}

	//Pfad erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, username, strlen(username)-OVERFLOW);

	//Benutzer nicht vorhanden
	if(access(mailpath, 00) == -1)
	{
		return -1;
	}

	strcat(mailpath, "/");
	strncat(mailpath, msg, strlen(msg)-OVERFLOW);
	strcpy(attachpath,mailpath);
	strcat(attachpath,".attach");


	//Mail löschen
	if(remove(mailpath) != 0)
		return -1;

    //Attachment löschen
	if(remove(attachpath) != 0)
		return -1;

	//Speicher freigeben
	free(msg);
	return 0;
}
