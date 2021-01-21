/* 
 * File:   sender_main.c
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
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

struct sockaddr_in si_other;
int s, slen;

#define SLOWSTART 73
#define CONGAVOID 74
#define FASTRECOV 75

#define min(x, y) x < y ? x : y

int state = SLOWSTART;

#define MAXWINSIZE 256
#define MAXPACSIZE 1464
#define TIMELIMIT 85000
float CW;
float SST;
int dupACK;

int isDup = 0;
int isNew = 0;

int timeout = 0;

typedef struct Pkt{
    int seq_num;
    int length;
    char content[MAXPACSIZE];
} Pkt_t;



int sentbuffer[MAXWINSIZE]; // record whether the packet is sent or not
Pkt_t packetbuffer[MAXWINSIZE];
char contentbuffer[MAXWINSIZE][MAXPACSIZE];


int nextByte = 0;
int expectedACK = 0;
//int bytes = 0;

int received_ACK[1];


void diep(char *s) {
    perror(s);
    exit(1);
}



int sendpacket(FILE* fp, unsigned long long int bytesToTransfer){
    //int numbytes;
    for(int i = expectedACK; i<(expectedACK+(int)CW); i++){
        if (sentbuffer[i%MAXWINSIZE] == 0){
            if (nextByte >= bytesToTransfer){
                return 0;
            }
            fread(packetbuffer[i%MAXWINSIZE].content, 1, min(MAXPACSIZE, bytesToTransfer - nextByte), fp);
            packetbuffer[i%MAXWINSIZE].seq_num = i;
            packetbuffer[i%MAXWINSIZE].length = min(MAXPACSIZE, bytesToTransfer - nextByte);
            //bytes += packetbuffer[i%MAXWINSIZE].length;
            nextByte += MAXPACSIZE;
	    //numbytes = sendto(s, &(packetbuffer[i % MAXWINSIZE]), sizeof(Pkt_t), 0, (struct sockaddr*)&si_other, slen);
            //sentbuffer[i%MAXWINSIZE] = 1;
	    //printf("nextByte:%d\n", nextByte);
        }
    }

    if (timeout == 1){
	sendto(s, &(packetbuffer[expectedACK % MAXWINSIZE]), sizeof(Pkt_t), 0, (struct sockaddr*)&si_other, slen);
    }
    else {
        for (int i = expectedACK; i <(expectedACK + (int)CW); i++){
            if (sentbuffer[i%MAXWINSIZE] == 0){
                sendto(s, &(packetbuffer[i % MAXWINSIZE]), sizeof(Pkt_t), 0, (struct sockaddr*)&si_other, slen);
                sentbuffer[i%MAXWINSIZE] = 1;
            }
        }
    }
    return 1;

}



void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    //Open the file
    //int numbytes;
    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }

	/* Determine how many bytes to transfer */

    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    CW = 1.0;
    SST = 64.0;
    dupACK = 0;
    state = SLOWSTART;
    timeout = 0;
    isDup = 0;
    isNew = 0;
    for (int i = 0; i < MAXWINSIZE; i++){
        contentbuffer[i][0] = '\0';
    }

    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = TIMELIMIT;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) < 0) {
        perror("sender: setsockopt");
    }
    sendpacket(fp, bytesToTransfer);

    while (1)
    {
        if((recvfrom(s, received_ACK, sizeof(int), 0, (struct sockaddr *) &si_other, (socklen_t*) &slen)) == -1){
            timeout = 1;            
        }
        else{
	    //printf("received_ACK = %d\n",received_ACK[0]);
            if(received_ACK[0]==(int)(bytesToTransfer/MAXPACSIZE)) break;
            timeout = 0;
            if (received_ACK[0] < expectedACK - 1){
                continue;
            }
            else if (received_ACK[0] >= expectedACK){
                isNew = 1;
                for (int i = expectedACK; i <= received_ACK[0]; i ++){
                    sentbuffer[i%MAXWINSIZE] = 0;
                }
                expectedACK = received_ACK[0] + 1;
            }
            else{
                isDup = 1;
            }
            
            
        }

        if (timeout){
            isDup = 0;
            isNew = 0;
        }

        if (state == SLOWSTART){
            if (isNew){
                if (CW < MAXWINSIZE){
                    CW = CW + 1.0;
                }
                sendpacket(fp, bytesToTransfer);
                dupACK = 0;
            }

            if (isDup){
                dupACK = dupACK + 1;
                if (dupACK >= 3){
                    SST = CW/2.0;
                    CW = SST + 3.0;
                    sendpacket(fp, bytesToTransfer);
                    state = FASTRECOV;
                }
            }

            if (timeout){
                SST = CW/2.0;
                CW = 1.0;
                dupACK = 0;
                sendpacket(fp, bytesToTransfer) ;
            }

            if (CW >= SST){
                state = CONGAVOID;
            }
            isDup = 0;
            isNew = 0;
            timeout = 0;
            continue;
        }
        
        if (state == CONGAVOID){
            if (isNew){
                if (CW < MAXWINSIZE){
                    CW = CW + 1.0/(int)CW;
                }
                dupACK = 0;
                sendpacket(fp, bytesToTransfer);
            }

            if (isDup){
                dupACK = dupACK + 1;
                if (dupACK >= 3){
                    SST = CW/2.0;
                    CW = SST + 3.0;
                    sendpacket(fp, bytesToTransfer);
                    state = FASTRECOV;
                }
            }

            if (timeout){
                SST = CW/2.0;
                CW = 1.0;
                dupACK = 0;
                sendpacket(fp, bytesToTransfer);
                state = SLOWSTART;
            }
            isDup = 0;
            isNew = 0;
            timeout = 0;
            continue;
        }

        if (state == FASTRECOV){
            if (isNew){
                CW = SST;
                dupACK = 0;
                sendpacket(fp, bytesToTransfer);
                state = CONGAVOID;
            }

            if (isDup){
                if (CW < MAXWINSIZE){
                    CW = CW + 1.0;
                }
                sendpacket(fp, bytesToTransfer);
            }

            if (timeout){
                SST = CW/2.0;
                CW = 1.0;
                dupACK = 0;
                sendpacket(fp, bytesToTransfer) ;
                state = SLOWSTART;
            }

            isDup = 0;
            isNew = 0;
            timeout = 0;
            continue;
        }
    }
    //printf("bytes = %d",bytes);
    //printf("next bytes = %d",nextByte);
    Pkt_t pktend;
    pktend.length = -1;
    int success[1];
    success[0] = 0;
    while(1){
        sendto(s, &pktend, sizeof(Pkt_t), 0, (struct sockaddr*)&si_other, slen);
        recvfrom(s, success, sizeof(int), 0, (struct sockaddr*) &si_other, (socklen_t*) &slen);
        if (success[0] == -1){
            break;
        }
    }

	/* Send data and receive acknowledgements on s*/

    //printf("Closing the socket\n");
    close(s);
    fclose(fp);
    return;

}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);



    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}



