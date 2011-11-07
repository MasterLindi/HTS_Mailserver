#include "serverfunctions.h"

//Socket
#define BUF 1024
#define OVERFLOW 1

//LDAP
#define LDAP_HOST "ldap.technikum-wien.at"
#define LDAP_PORT 389
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE

#define BIND_USER "uid=if10b015,ou=People,dc=technikum-wien,dc=at"		/* anonymous bind with user and pw NULL */
#define BIND_PW "1mauzer1"

//Mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

//Benutzer einloggen
int loginuser(int socket ,char **username)
{
	int size;
	int i;
	int j;
	int len;
	char buffer[BUF];
	char *user;
	char *pw;

	if(*username != NULL) //Benutzer bereits eingeloggt
		return -1;


	for(i = 0; i < 2; i++)
	{
		size = readline(socket, buffer, BUF-1);   //Lesen von Client

		if(size > 0)
		{
			buffer[size] = '\0';

			switch(i)
			{
				case 0: //Benutzername auslesen
					user = (char *)malloc(strlen(buffer)+1);
					strcpy(user, buffer);

					len = strlen(user);

					if(len-OVERFLOW > 8) //Länge des Sender größer 8, wenn ja Fehler
						return -1;

					//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++)
					{
						if(!isalnum(user[j]))   //Prüfe ob nur erlaubte Zeichen vorkommen
						{
							free(user);
							return -1;
						}
					}
					break;

				case 1: //Passwort auslesen
					pw = (char *)malloc(strlen(buffer)+1);
					strcpy(pw, buffer);
					pw[strlen(pw)-1] = '\0';        //\n wird aus Passwort entfernt
					break;
			}
		}
		else
		{
			return -1;
		}


	}

	if(ldapauth(user, pw) == 0)     //Überprüfen ob Ldap-Authentifikation erfolgreich ist
	{
		*username = (char *)malloc(strlen(user)+1);
		strcpy(*username, user);
	}
	else
		return -1;

	free(user); //Speicher freigeben
	free(pw);

	return 0;
}

int ldapauth(char *user, char *password)
{
   LDAP *ld;			/* LDAP resource handle */
   LDAPMessage *result, *e;	/* LDAP result handle */
   BerElement *ber;		/* array of attributes */
   char *attribute;
   char filter[100];
   char *dn;
   char **vals;

   int i,rc=0;

   char *attribs[3];		/* attribute array for search */

   attribs[0]=strdup("uid");		/* return uid and cn of entries */
   attribs[1]=strdup("cn");
   attribs[2]=NULL;		/* array must be NULL terminated */


   strcpy(filter, "(uid=");
   strncat(filter, user, strlen(user)-OVERFLOW);
   strcat(filter, ")");

   /* setup LDAP connection */
   if ((ld=ldap_init(LDAP_HOST, LDAP_PORT)) == NULL)
   {
      perror("ldap_init failed");
      return -1;
   }

   printf("LDAP: connected to LDAP server %s on port %d\n",LDAP_HOST,LDAP_PORT);

   /* anonymous bind */
   rc = ldap_simple_bind_s(ld,BIND_USER,BIND_PW);

   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr,"LDAP error: %s\n",ldap_err2string(rc));
      return -1;
   }
   else
   {
      printf("LDAP: bind successful\n");
   }

   /* perform ldap search */
   rc = ldap_search_s(ld, SEARCHBASE, SCOPE, filter, attribs, 0, &result);

   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr,"LDAP search error: %s\n",ldap_err2string(rc));
      return -1;
   }


   if(ldap_count_entries(ld, result) == 1)
   {
	   e = ldap_first_entry(ld, result);
	   printf("LDAP: DN: %s\n", ldap_get_dn(ld,e));
	   dn = strdup(ldap_get_dn(ld,e));
   }
   else
   {
	   return -1;
   }

   /* free memory used for result */
   ldap_msgfree(result);
   free(attribs[0]);
   free(attribs[1]);

   ldap_unbind(ld);

   if(ldaplogin(dn, password) == -1)
   {
	   return -1;
   }

   return 0;
}

int ldaplogin(char *dn, char *pw)
{
	LDAP *ld;			/* LDAP resource handle */
   LDAPMessage *result, *e;	/* LDAP result handle */
   BerElement *ber;		/* array of attributes */
   char *attribute;
   char **vals;
   int i,rc=0;

   char *attribs[3];		/* attribute array for search */

   attribs[0]=strdup("uid");		/* return uid and cn of entries */
   attribs[1]=strdup("cn");
   attribs[2]=NULL;		/* array must be NULL terminated */

   /* setup LDAP connection */
   if ((ld=ldap_init(LDAP_HOST, LDAP_PORT)) == NULL)
   {
      perror("ldap_init failed");
      return -1;
   }

   printf("LDAP: connected to LDAP server %s on port %d\n",LDAP_HOST,LDAP_PORT);

   /* anonymous bind */
   rc = ldap_simple_bind_s(ld,dn,pw);

   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr,"LDAP error: %s\n",ldap_err2string(rc));
      return -1;
   }
   else
   {
      printf("LDAP: bind successful\n");
   }

   /* free memory used for result */
   ldap_msgfree(result);
   free(attribs[0]);
   free(attribs[1]);

   ldap_unbind(ld);
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
	int status;

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
	long totallength= 0,templength=0,count=0;

    	receiver = malloc(100*sizeof(char *)); //Empfänger array allokieren

	if(username == NULL)        //Überprüfen ob Benutzer eingeloggt ist
	{
		return -1;
	}
	else
		send(socket, "OK\n", 3, 0);

	do
	{
		size = readline(socket, buffer, BUF-1);     //Einlesen von Client

		if(size > 0)
		{
			buffer[size] = '\0';

			switch(i)
			{
				case 0: //Empfänger
				    if ((strncmp(buffer,"empf@aus",strlen("empf@aus")))== 0) break;
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
					send(socket, "OK\n", 3, 0); 
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

                    if ((strncmp(buffer, "noattach", strlen("noattach")))==0 && count ==0)  //Überprüfen ob ein Attachment gesendet wird
                    {
                        attach = 0;
                        count = 1;
                        i++;
                        break;
                    }
		    if (strncmp(buffer, "errattach", strlen("errattach")) == 0)  //Überprüfen ob ein Attachment gesendet wird
                    {
                        return -1;
                    }

			
                    if (count == 0)     //1. Parameter des Attachment ist die Länge es Files
                    {
                         totallength = strtol(buffer,NULL,10);
                         count = 1;
                         break;
                    }

                    if(templength == totallength)  //Ende des Attachment
					{
						break;
					}
					else //Einlesen des Inhaltes
					{
						if(attachment == NULL) //Wenn noch nichts gesendet wurde
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
                            				templength = templength + strlen(buffer);   //Mitzählen der einglesenen Bytes
						}
						else
						{
							len = strlen(attachment);   //Nachstehende zeilen werden eingelesen
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

							templength = templength + strlen(buffer);
						}

					}
					break;

				default: //Inhalt

					if(strncmp(buffer, ".", 1) == 0) //Ende der Nachricht
					{
						if(content == NULL)
						{
							content = (char *)malloc(2);
							strcpy(content, "\n");
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

							strcat(content, buffer); //Anhängen des Inhalts an die Nachricht
						}
					}
					break;
			}

		}
		else
		{
			return -1;
		}

		if ((strncmp(buffer,"empf@aus",strlen("empf@aus")))==0 || i > 0)    //Abfrage ob noch Empfänger kommen
		{   
			if ( (i==2) && (templength == totallength)) i++;        //Abfrage ob noch Bytes vom Attachment kommen
		        if (i!= 2) i++;
		}

	}
	while(quit == 0);

	status = pthread_mutex_lock(&mutex);
    	for ( i = 0; i<anzempf; i++)
    	{
        	//Verzeichnispfade erstellen
		strcpy(mailpath, spool);
		strcat(mailpath, "/");
		strncat(mailpath, receiver[i], strlen(receiver[i])-OVERFLOW);

		//Configfilepfad
		strcpy(confpath, mailpath);
		strcat(confpath, "/conf.ini");

		//Attachmentpfad
		strcpy(attachpath,mailpath);
		strcat(attachpath,"/attach/");  //Attachment wird in Unterordner abgelegt

		//Existiert das Verzeichnis bereits
		if(access(mailpath, 00) == -1)
		{
			mkdir(mailpath, 0777); //Verzeichnis für den Benutzer erstellen
			mkdir(attachpath, 0777); //Attachmentverzeichnis für den Benutzer erstellen
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
    		strcat (attachpath, bufid);

		//Email abspeichern
		if((fp = fopen(mailpath, "w")) == NULL)
		{
			status = pthread_mutex_unlock(&mutex);
			return -1;
		}

		fputs(username, fp);
		fputs(receiver[i], fp);
		fputs(subject, fp);
		fputs(content, fp);

		fclose(fp);

		//Attachment abspeichern
		if (attach != 0)
		{
		    if((fp = fopen(attachpath, "w")) == NULL) 
			{
				status = pthread_mutex_unlock(&mutex);
				return -1;
			}
		    fputs(attachment,fp);
		    fclose(fp);
		}

		//ID im Config-File erhöhen und in Datei schrieben
		id = strtol(bufid, NULL, 10);
		id ++;

		fp = fopen(confpath, "w");
		fprintf(fp, "%d", id);
		fclose(fp);

    	}
    	status = pthread_mutex_unlock(&mutex);

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

	//Prüfung, ob User eingeloggt
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
				strcmp((*p).d_name, "conf.ini") != 0 &&
				strcmp((*p).d_name, "attach") != 0

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

	if(username == NULL)    //Überprüfung ob wer eingeloggt ist
	{
		return -1;
	}
	else
		send(socket, "OK\n", 3, 0);

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
	strcpy(attachpath, mailpath);
	strncat(mailpath, msg, strlen(msg)-OVERFLOW);
	strcat(attachpath, "attach/");
	strncat(attachpath, msg, strlen(msg)-OVERFLOW);

	//Mail-File öffnen, Prüfung ob Mail vorhanden
	if((fp = fopen(mailpath, "r")) == NULL)
	{
		return -1;
	}

	//Prüfen ob Nachricht ein Attachment enthält
	if ((fpattach = fopen(attachpath,"r"))==NULL)
	{   
		strcpy (buffer,"false\n");
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
				long sizeoffile=0;
		        	struct stat attribut;
		        	if (stat(attachpath, &attribut) == -1)    //Attachmentgröße wird bestimmt
				{
					fprintf(stderr,"Dateifehler\n");
				}
				else
				{
					sizeoffile= attribut.st_size;
				}

				sprintf( buffer, "%ld", sizeoffile );
				strcat(buffer,"\n");
				send(socket, buffer, strlen(buffer), 0);

				while (fgets(buffer,BUF,fpattach))
				{
				    send(socket,buffer,strlen(buffer),0);
				}                
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
	int status;
	FILE *fp;
	char buffer[BUF];
	char mailpath[150];
	char attachpath[150];
	char *msg = NULL;

	//Prüfung, ob User eingeloggt
	if(username == NULL)
	{
		return -1;
	}
	else
		send(socket, "OK\n", 3, 0);

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

	status = pthread_mutex_lock(&mutex);
	//Pfad erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, username, strlen(username)-OVERFLOW);

	//Benutzer nicht vorhanden
	if(access(mailpath, 00) == -1)
	{
		status = pthread_mutex_unlock(&mutex);
		return -1;
	}

	strcat(mailpath, "/");
        strcpy(attachpath,mailpath);
	strncat(mailpath, msg, strlen(msg)-OVERFLOW);
	strcat(attachpath,"attach/");       //dazugehöriges Attachment wird ebenfalls gelöscht
	strncat(attachpath, msg, strlen(msg)-OVERFLOW);

	//Mail löschen
	if(remove(mailpath) != 0)
	{
		status = pthread_mutex_unlock(&mutex);
		return -1;
	}

        //Attachment löschen
	if(access(attachpath, 00) == 0)
	{
		if(remove(attachpath) != 0)
		{
			status = pthread_mutex_unlock(&mutex);
			return -1;
		}
	}
	status = pthread_mutex_unlock(&mutex);

	//Speicher freigeben
	free(msg);
	return 0;
}
