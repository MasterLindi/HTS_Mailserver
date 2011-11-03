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


//Mail senden
int sendmail(int socket, char *spool)
{
	int size; 
	int i = 0;
	int j;
	int len = 0;
	int quit = 0;
	int id = 0;
	
	char bufid[33];
	FILE *fp = NULL;
	char buffer[BUF];
	char mailpath[150];	
	char confpath[150];	
	char *sender = NULL;
	char *receiver = NULL;
	char *subject = NULL;
	char *content = NULL;
		
	do
	{
		size = readline(socket, buffer, BUF-1); 
		
		if(size > 0)
		{
			buffer[size] = '\0';
			
			switch(i)
			{
				case 0: //Sender
					sender = (char *)malloc(strlen(buffer)+1);
					strcpy(sender, buffer);
							
					len = strlen(sender);
					
					if(len-OVERFLOW > 8) //Länge des Sender größer 8, wenn ja Fehler
						return -1;		
					
					//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++) 
					{				
						if(!isalnum(sender[j]))
						{
							free(sender);
							return -1;
						}
					}
					break;
				
				case 1: //Empfänger
					receiver = (char *)malloc(strlen(buffer)+1);
					strcpy(receiver, buffer);
					
					len = strlen(receiver);
					
					if(len-OVERFLOW > 8) //Länge des Empfängers größer 8, wenn ja Fehler
						return -1;	
						
					//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++)
					{				
						if(!isalnum(receiver[j]))
						{
							free(sender);
							free(receiver);
							return -1;
						}
					}
					break;
				
				case 2: //Betreff
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
							free(sender);
							free(receiver);
							free(subject);
							return -1;
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
								free(sender);
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
								free(sender);
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
		i++;
	}
	while(quit == 0);
	
	
	//Verzeichnispfad erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, receiver, strlen(receiver)-OVERFLOW);
	strcpy(confpath, mailpath);
	strcat(confpath, "/conf.ini");
		
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
		
	//Email abspeichern
	if((fp = fopen(mailpath, "w")) == NULL)
		return -1;
	
	fputs(sender, fp);
	fputs(receiver, fp);
	fputs(subject, fp);
	fputs(content, fp);
	
	fclose(fp);
	
	//ID im Config-File erhöhen und in Datei schrieben
	id = strtol(bufid, NULL, 10);
	id ++;
	
	fp = fopen(confpath, "w");
	fprintf(fp, "%d", id);
	fclose(fp);
	
	//Speicher freigeben
	free(sender);
	free(receiver);
	free(subject);
	free(content);
	
	return 0;
}

//Emails eines Benutzers anzeigen
int listmail(int socket, char *spool)
{
	int size; 
	int i;
	char buffer[BUF];
	char subject[81];
	char mailpath[150];
	char msgpath[150];	
	FILE *fp = NULL;
	DIR *ver;
	struct dirent *p;
	
	size = readline(socket, buffer, BUF-1);
		
	if(size > 0)
	{
		buffer[size] = '\0';
		
		//Pfad zum Benutzer erstellen
		strcpy(mailpath, spool);
		strcat(mailpath, "/");
		strncat(mailpath, buffer, strlen(buffer)-OVERFLOW);
		
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
			
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int readmail(int socket, char *spool)
{
	int size;
	int i;
	int j;
	int len;
	FILE *fp;
	char buffer[BUF];
	char mailpath[150];
	char *user = NULL;
	char *msg = NULL;
	
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
					
				case 1: //Nachricht
					msg = (char *)malloc(strlen(buffer)+1);
					strcpy(msg, buffer);
							
					len = strlen(msg);
					
					//Prüfung ob alle Zeichen alphanumerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++) 
					{				
						if(!isdigit(msg[j]))
						{
							free(user);
							free(msg);
							return -1;
						}
					}
					break;
			}
		}
		else
		{
			return -1;
		}
	}
	
	//Pfad erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, user, strlen(user)-OVERFLOW);	
	
	//Benutzer nicht vorhanden
	if(access(mailpath, 00) == -1)	
	{
		return -1;
	}
	
	strcat(mailpath, "/");
	strncat(mailpath, msg, strlen(msg)-OVERFLOW);
	
	//Mail-File öffnen, Prüfung ob Mail vorhanden
	if((fp = fopen(mailpath, "r")) == NULL)
	{
		return -1;
	}
		
	//Gewählte Mail an Client schicken
	while(fgets(buffer, BUF, fp))
	{
	    send(socket, buffer, strlen(buffer),0);
	}
	
	//Mail-File schließen			
	fclose(fp);
	
	//Speicher freigeben
	free(user);
	free(msg);
				
	return 0;
}

int delmail(int socket, char *spool)
{
	int size;
	int i;
	int j;
	int len;
	FILE *fp;
	char buffer[BUF];
	char mailpath[150];
	char *user = NULL;
	char *msg = NULL;
	
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
					
				case 1: //Nachricht
					msg = (char *)malloc(strlen(buffer)+1);
					strcpy(msg, buffer);
							
					len = strlen(msg);
					
					//Prüfung ob alle Zeichen numerisch sind, wenn nicht, Fehler
					for(j = 0; j < len-OVERFLOW; j++) 
					{				
						if(!isdigit(msg[j]))
						{
							free(user);
							free(msg);
							return -1;
						}
					}
					break;
			}
		}
		else
		{
			return -1;
		}
	}
	
	//Pfad erstellen
	strcpy(mailpath, spool);
	strcat(mailpath, "/");
	strncat(mailpath, user, strlen(user)-OVERFLOW);	
	
	//Benutzer nicht vorhanden
	if(access(mailpath, 00) == -1)	
	{
		return -1;
	}
	
	strcat(mailpath, "/");
	strncat(mailpath, msg, strlen(msg)-OVERFLOW);
	
	//Mail löschen
	if(remove(mailpath) != 0)
		return -1;
		
	//Speicher freigeben
	free(user);
	free(msg);
	return 0;
}
