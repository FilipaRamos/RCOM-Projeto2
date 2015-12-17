#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <signal.h>

#define SERVER_PORT 21

struct 	ftp_parsed{

 char ipAdress[15];	
 char user[255];
 char password[255];
 char path[255];
 char host[255];
 int sockfd;
 int secSockfd;

};
