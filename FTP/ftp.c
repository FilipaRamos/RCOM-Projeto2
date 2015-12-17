#include "ftp.h"

struct ftp_parsed  * ftp_parsed;

int valideteUrl(char * url){
    	regex_t regex;

    	//confirmar expressão 
       char * expression = "ftp://[[A-Za-z0-9]+:[A-Za-z0-9]+@]*[A-Za-z0-9._~:?#@!$&'()*+,:;=-]+/[A-Za-z0-9._~:/?#@!$&'()*+,:;=-]+";

		//compile the regular expression contained in the string pointed to by the expression
    	int res = regcomp(&regex, expression, REG_EXTENDED);

    	if(res == REG_NOMATCH){
    		printf("Invalid URL\n");
    		return 1;
    	}

    	if(res != 0){
    		perror("Error in URL\n");
    		return 1;
    	}

    		res = regexec(&regex, url, 0, NULL, 0);
    	regfree(&regex);

    	if(res ==  REG_NOMATCH){
    		printf("no match found\n");
    		return(1);
    	}

    	if(res == REG_ENOSYS){
    		printf("function is not supported\n");
    		return(1);
		}

    	if (res != 0) {
    		perror("Error exec\n");
        	return(1);      /* report error */
    	}

    	return(0);

}

//function that recives char * to save user, password, host and path
int parseUrl(char* url, char* host, char* user, char* password, char* path){

	//ftp://[<user>:<password>@]<host>/<url-path>

  char *  ftp = url + 6;

  printf("%s\n", url);
  printf("%s\n", ftp);
  printf("entrou\n");

	char * res = strtok(ftp, ":");

		user = res;
    strcpy(ftp_parsed->user, user);

	printf("%s\n", ftp_parsed->user);

  res = strtok(NULL, "@");

		password = res;
    strcpy(ftp_parsed->password, password);
  
  printf("%s\n", ftp_parsed->password);


	res = strtok(NULL, "/");

	host = res;
  strcpy(ftp_parsed->host, host);

	printf("%s\n", ftp_parsed->host);

	const char * res1 = strtok(NULL, "/");

  while(res1 != NULL){
    strcat(path, res1);
    strcat(path, "/");
    res1 = strtok(NULL, "/");
  }
  path[strlen(path)-1] = '\0';
	printf("%s\n", path);

  strcpy(ftp_parsed->path,path);

	return 0;
}

int getIpAdress(char * host, char * buf){
  struct hostent *h;

    if ((h=gethostbyname(host)) == NULL) {  
        herror("gethostbyname");
        return -1;
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

    strcpy(buf, inet_ntoa(*((struct in_addr *)h->h_addr)));

    return 0;

}

int connect_socket(const char* ip, const int port){
  int sockfd;
  struct  sockaddr_in server_addr;
  
  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(SERVER_PORT);    /*server TCP port must be network byte ordered */
    
  /*open an TCP socket*/
  if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket()");
         return -1;
      }
  /*connect to the server*/
      if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
          perror("connect()");
  }

 return sockfd;

}

int send_command(struct  ftp_parsed* ftp, const char* command, const int size){

//resposta do write ou é 0 ou é o nºbytes enviados enviado
  int res = write(ftp->sockfd, command, size);

  if(res <= 0){
    printf("Error: no information sent to server\n");
    return -1;
  }

  if(res != size){
    printf("Error: some information sent to server\n");
    return -1;
  }

  return 0;

}


int read_res_server(int sockfd, char* buf, int size){

  int res = read(sockfd, buf, size);
  buf[res] = 0;

  printf("%s", buf);
  
  return res;

}

int log_in(char* user, char*password, int sockfd){

  char buf[255];
  
  read_res_server(sockfd, buf, 255);

  sprintf( buf, "user %s\n", user);
  write(sockfd, buf, strlen(buf));

  read_res_server(sockfd, buf, 255);

  sprintf(buf, "pass %s\n", password);
  write(sockfd, buf, strlen(buf));

  printf("%s", buf);
  read_res_server(sockfd, buf, 255);

  int res;
  sscanf(buf, "%d", &res);
  //fail login
  if(res == 530){
    printf("Wrong username/password.\n");
    return -1;
  }

  return 0;
}

int quit(struct ftp_parsed* ftp){
  write(ftp->sockfd, "quit\n", strlen("quit\n"));
  close(ftp->sockfd);
  return 0;
}

int pasv(struct  ftp_parsed* ftp){
  char buf[255];
  int ip1, ip2, ip3, ip4, port, port1, port2;
  char bufIp[15];

  send_command(ftp, "pasv\n", strlen("pasv\n"));
  read_res_server(ftp->sockfd, buf, 255);

  int res = sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2);

  if(res < 0){
    printf("error getting variables from passive mode.\n");
  }

  sprintf(bufIp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

  port = port1*256 + port2;

  printf("bufIp %s\n", bufIp);
  printf("port %d\n", port);

  ftp->secSockfd = connect_socket(bufIp, port);
  printf("secSockfd: %d\n", ftp->secSockfd);
  return 0;
}

int retr(struct  ftp_parsed* ftp){
  char buf[255];

  sprintf(buf, "retr %s\n", ftp->path);

  printf("buf: %s\n", buf);

  int i = send_command(ftp, buf, strlen(buf));
  printf("%d\n", ftp->sockfd);
  //read_res_server(ftp->sockfd, buf, 255);
  printf("saddsa\n");
  return 0;
}

int download(struct  ftp_parsed* ftp){
  char buf[255];
  int bytesRead = 0;
  char filename[255];

  strcpy(filename, ftp->path);
  FILE * file = fopen(filename, "w");

  if(file == NULL){
    printf("Can not open file for writing.\n");
    return -1;
  }

  while( (bytesRead = read(ftp->secSockfd, buf, sizeof(buf)) )!= 0){
    if(bytesRead < 0){
      return -1;
    }

    if(fwrite(buf,sizeof(char),bytesRead,file) == 0){
      if(ferror(file) != 0){
        printf("Error in writing to file %s\n", filename);
        return -1;
      }
    }
  }

  printf("File downloaded \n");
  fclose(file);
  return 0;
}

int main(int argc, char** argv)
{
  if(argc != 2){
    printf("numero de argumentos errado. \n");
    printf("%s (porta(/dev/ttySN)) ficheiro flag(1-transmitter, 0-receiver) \n", argv[0]);
    return 0;
  }

  char* host = "";
  char* user = "";
  char* password = "";
  char* path = malloc(sizeof(char) * 1024);
  char* buf = malloc(sizeof(char) * 1024);
  ftp_parsed = malloc(sizeof(struct ftp_parsed));

  int res = parseUrl(argv[1],host,user,password,path);

  printf("%s\n",ftp_parsed->host);

  getIpAdress(ftp_parsed->host,buf);
  strcpy(ftp_parsed->ipAdress ,buf);

  //printf("%s\n", user);
  printf("FTP MAIN: PARSE URL CHECK\n");
  printf("FTP MAIN: CONNECT START\n");
  ftp_parsed->sockfd = connect_socket( buf, 21);

  printf("FTP MAIN: CONNECT CHECK\n");
  printf("FTP MAIN: LOGIN START\n");
  log_in(ftp_parsed->user, ftp_parsed->password, ftp_parsed->sockfd);

  printf("FTP MAIN: LOGIN CHECK\n");
  printf("FTP MAIN: PASV START\n");
  pasv(ftp_parsed);

  printf("FTP MAIN: PASV CHECK\n");
  printf("FTP MAIN: RETR START\n");
  retr(ftp_parsed);

  printf("FTP MAIN: RETR CHECK\n");
  printf("FTP MAIN: DOWNLOAD START\n");
  download(ftp_parsed);

  printf("FTP MAIN: DOWNLOAD CHECK\n");
  printf("FTP MAIN: QUIT START\n");
  quit(ftp_parsed);

  printf("FTP MAIN: QUIT CHECK\n");

  return res;

}
