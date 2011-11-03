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

#ifndef serverfunctions_h
#define serverfunctions_h

int sendmail(int, char *);
int listmail(int, char *);
int readmail(int, char *);
int delmail(int, char *);

#endif 
