/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getaddrfile(char* arg1, char* addr, char*port, char*file){
	char arg[strlen(arg1)];
	char arg2[strlen(arg)];
	int maohao = 0;
	if (strncmp(arg1, "http://",7) == 0){
		strcpy(arg, arg1+7);
	}
	else{
		strcpy(arg, arg1);	
	}
	for (int i = 0; i < strlen(arg); i++){
		if(strncmp(arg+i, ":", 1) == 0){
			strncpy(addr, arg, i);
			addr[i] = '\0';
			strcpy(arg2, arg+i+1);
			maohao = 1;
			break;
		}
	}
	if (maohao == 0){
		strcpy(arg2, arg);
	}
	for (int j = 0; j < strlen(arg2); j++){
		if(strncmp(arg2+j, "/",1) == 0){
			if (maohao == 1){
				strncpy(port, arg2, j);
				port[j] = '\0';
			}
			else{
				strncpy(addr, arg2, j);
				addr[j] = '\0';
				port[0] = '8';
				port[1] = '0';
				port[2] = '\0';
			}
			strcpy(file+1, arg2+j+1);
			file[0] = '/';
			break;
		}
	}
	
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	char buf1[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	
	char port[10];
	char addr[100];
	char file[100];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}
	
	char arg1[strlen(argv[1])];
	strcpy(arg1, argv[1]);
	getaddrfile(arg1, addr, port, file);
	
	printf("addr: %s\n", addr);
	printf("port: %s\n", port);
	printf("file: %s\n", file);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(addr, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	//send request
	memset(buf, 0, MAXDATASIZE);
	sprintf(buf,"GET %s HTTP/1.1\r\nUser-Agent: Wget/1.12 (linux-gnu)\r\nHost: localhost:%s\r\nConnection: Keep-Alive\r\n\r\n",file,port);
	send(sockfd,(char*)buf,MAXDATASIZE,0);


	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure


	

/************************************************************************************************************************/
// delete header. store to outputfile
	int flag = 0;
	char * start;
	FILE * file_ = fopen("output","wb");
	memset(buf1, 0, MAXDATASIZE);
	while(1) {
		if((numbytes = recv(sockfd, buf1, MAXDATASIZE-1, 0)) > 0){
			start = strstr(buf1,"\r\n\r\n") + 4; //after the head
			if (flag==0) {
				if(start == NULL){
					continue;
				}
				fwrite(start, sizeof (char), numbytes + buf1 - start, file_);
				flag = 1;
				printf("%s \n", start);
		   // numbytes = numbytes + buf1 - start;
			} else {
				fwrite(buf1, sizeof (char), numbytes, file_);
				printf("%s \n", buf1);
			}
		}	
		else{
			break;
		}  
	} 
	fclose(file_);

	close(sockfd);
/************************************************************************************************************************/
	return 0;
}

