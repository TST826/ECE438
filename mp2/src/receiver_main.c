/* 
 * File:   receiver_main.c
 * Author: 
 *
 * Created on
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT "3490"
#define MAXWINSIZE 256
#define MAXPACSIZE 1464

struct sockaddr_in si_me, si_other;
int s, slen;

typedef struct Pkt{
    int seq_num;
    int length;
    char content[MAXPACSIZE];
} Pkt_t;

Pkt_t packetbuffer[MAXWINSIZE];

Pkt_t currpkt;

int got_seq[0];
int received_buffer[MAXWINSIZE];

int lastconfirmed_ACK;
int ACK_temp;
int bytesrec=0;
//int bytes=0;
void diep(char *s) {
    perror(s);
    exit(1);
}

void sendACK(){
    ACK_temp = lastconfirmed_ACK;
    while (received_buffer[(ACK_temp + 1) % MAXWINSIZE] == 1){
        ACK_temp = (ACK_temp+1);
    }
    sendto(s, &ACK_temp, sizeof(int), 0, (struct sockaddr*)&si_other, slen);
}

void writetofile(FILE* fp){
    while (received_buffer[(lastconfirmed_ACK + 1) % MAXWINSIZE] == 1){
        lastconfirmed_ACK = (lastconfirmed_ACK+1);
        int c = fwrite(packetbuffer[lastconfirmed_ACK % MAXWINSIZE].content, 1, packetbuffer[lastconfirmed_ACK % MAXWINSIZE].length, fp);
        //bytes+= packetbuffer[lastconfirmed_ACK % MAXWINSIZE].length;
        received_buffer[lastconfirmed_ACK % MAXWINSIZE] = 0;
    }
}


void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    
    slen = sizeof (si_other);


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    //printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");

    FILE* fp = fopen(destinationFile, "wb");
    lastconfirmed_ACK = -1;

    while(1){
        recvfrom(s, &currpkt, sizeof(Pkt_t), 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);
	//printf("currpkt.length = %d\n",currpkt.length);
        //printf("currpkt.content = %s\n",currpkt.content);
        if (currpkt.length == -1){
            int finish = -1;
            sendto(s,  &finish, sizeof(int), 0, (struct sockaddr*)&si_other, slen);
            break;
        }
        got_seq[0] = currpkt.seq_num;
        //printf("got_seq[0] = %d",got_seq[0]);
        if (got_seq[0] >= lastconfirmed_ACK){
            received_buffer[got_seq[0]%MAXWINSIZE] = 1;
            packetbuffer[got_seq[0]%MAXWINSIZE] = currpkt;
	    sendACK();
            writetofile(fp);
            //printf("lastconfirmed_ACK = %d",lastconfirmed_ACK);
        }
	else{
	    sendto(s, &lastconfirmed_ACK, sizeof(int), 0, (struct sockaddr*)&si_other, slen);
	}

	//printf("lastconfirmed_ACK___ = %d\n\n\n",lastconfirmed_ACK);
    }
	/* Now receive data and send acknowledgements */    
    //printf("bytes = %d",bytes);
    close(s);
    fclose(fp);
    //printf("%s received.", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}


