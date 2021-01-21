/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

//#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 1024
#define MAXFILE 1024
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;


	//char buffer[MAXDATASIZE];
/////////////////////////////////////////////////////////////
	if (argc != 2) {
	    fprintf(stderr,"usage: hostport\n");
	    exit(1);
	}
/////////////////////////////////////////////////////////////

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
/************************************************************************************************************************/
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			//receive request
			char request[MAXDATASIZE];//request
			char file_path[MAXDATASIZE];
			memset(request, 0, MAXDATASIZE);
			memset(file_path, 0, MAXDATASIZE);
			rv = recv(new_fd, request,MAXDATASIZE, 0);
			printf("rv:%d",rv);
			char cmd[100];
			memset(cmd, 0, 100);
			cmd[0] = request[0];
			cmd[1] = request[1];
			cmd[2] = request[2];
			cmd[3] = '\0';
			if (strncmp(cmd, "GET", 3) != 0)	{		// This server only supports GET
				send(new_fd,"400 Bad Request",15,0);
				exit(0);
			}
			//get file path name
			char* start_path = strchr(request,'/');
			start_path++;
			char* end_path = strchr(request, '\n');
			printf("this is start path : %s\n", start_path);
			while(*end_path != ' '){
				end_path--;
			}
			strncpy(file_path, start_path, end_path - start_path);
			printf("%s\n", file_path);
                        //return file to client
			char line[MAXFILE];
			FILE *file_ = fopen(file_path, "rb");
			if(file_ ==NULL){
				send(new_fd,"HTTP/1.1 404 Not Found\r\n",strlen("HTTP/1.1 404 Not Found\r\n") , 0);
			}
			
			else{
			  send(new_fd,"HTTP/1.1 200 OK\r\n\r\n",strlen("HTTP/1.1 200 OK\r\n\r\n"),0);
			  while(fgets(line,MAXFILE-1,file_)!=NULL){
			    line[strlen(line)]='\0';
			    send(new_fd,line,strlen(line),0);
		            
			  }
			  fclose(file_);
			
			}

			close(new_fd);  //
			exit(0);

		}
		close(new_fd);  //
	}
/************************************************************************************************************************/
	return 0;
}
