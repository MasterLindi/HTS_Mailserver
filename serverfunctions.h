#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>

#ifndef functions_h
#define functions_h

ssize_t readline (int , void *, size_t);
int loginuser(int, char **);
int sendmail(int, char *, char *);
int listmail(int, char *, char *);
int readmail(int, char *, char *);
int delmail(int, char *, char *);

#endif 
