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
#include <termios.h>

#ifndef clientfunctions_h
#define clientfunctions_h

void quitcom(int, char*);
void sendcom(int, char*);
void listcom(int, char*);
void readcom(int, char*);
void delcom(int, char*);
void logincom(int,char*);

#endif
