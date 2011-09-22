// Multicast slinger
// Change: IP_MULTICAST_LOOP : Enable / Disable loopback for outgoing messages
// 
// Compile : gcc -o mslinger mslinger.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define MAXBUFSIZE 65536 // Max UDP Packet size is 64 Kbyte

static int cc[8192];
int inspect(unsigned char* tsbuf/*, FILE *out*/){
    if(tsbuf[0] != 0x47){
        printf("Wrong syncbyte!\n");
    }
    int payload = -1;
    unsigned int pid;
    pid = (tsbuf[1] & 0b00011111);
    pid = pid << 8;
    pid =  pid + tsbuf[2];
    if(pid == 8191)
        return 0;
/*    if(pid == 263)
        return 1;
    if(pid == 264)
        return 1;
    if(pid == 576)
        return 0;
    if(pid == 0)
        return 0;
    if(pid == 17)
        return 0;
    if(pid == 34)
        return 0;
    return 0; */
//    printf("PID: %d\n", pid);
    if(cc[pid] == -1) {
      cc[pid] = tsbuf[3] & 0b00001111;
    } else {
      cc[pid] = (cc[pid] + 1) & 0b00001111;
//      printf("BBQ: %d ", cc[pid]);
      if (cc[pid] != (tsbuf[3] & 0b00001111)){
        printf("WRONG CC! Pid:%d  %d != %d\n", pid, cc[pid], (tsbuf[3] & 0b00001111));
        cc[pid] = tsbuf[3] & 0b00001111;
      }
    }
/*    printf("cc: %d ", cc[pid]);
    if(tsbuf[1] & 0b10000000){
        printf("TEI ");
    }
    if(tsbuf[1] & 0b01000000){
        printf("PUSI ");
    }
    if(tsbuf[1] & 0b00100000){
        printf("PRI ");
    }
    if(tsbuf[3] & 0b11000000){
        printf("SCR ");
    }
    if(tsbuf[3] & 0b00100000){
        if(tsbuf[3] & 0b00010000){
          printf("ADA-PAY ");
          payload = 1;
        }else{
          printf("ADA-ONLY ");
        }
    } else {
        payload = 4;
    }
    if(payload == 1){
        int AFL = tsbuf[4];
        printf("AFL: %d ",AFL);
        payload += AFL+1;
    }

    if(payload != -1){
        int size = fwrite(tsbuf+payload, 1, 188-payload, out);
        printf("write: %d ", size);
    } */
//    printf("\n");
    return 1;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    int i;
    for(i = 0; i < 8192; i++)
        cc[i] = -1;
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
   int sock, status, socklen;
   char buffer[MAXBUFSIZE];
   char bufferc[MAXBUFSIZE];
   struct sockaddr_in saddr;
   struct ip_mreq imreq;

   // set content of struct saddr and imreq to zero
   memset(&saddr, 0, sizeof(struct sockaddr_in));
   memset(&imreq, 0, sizeof(struct ip_mreq));

   // open a UDP socket
   sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
   if ( sock < 0 )
     perror("Error creating socket"), exit(0);

   saddr.sin_family = PF_INET;
   saddr.sin_port = htons(atoi(argv[3])); // listen on port...
   saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface
   status = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
   printf("bound to multicast socket\n");
   

   if ( status < 0 )
     perror("Error binding socket to interface"), exit(0);

   imreq.imr_multiaddr.s_addr = inet_addr(argv[2]);
   //imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface
   imreq.imr_interface.s_addr = inet_addr("129.240.203.149"); // use DEFAULT interface

   // JOIN multicast group on default interface
   status = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
              (const void *)&imreq, sizeof(struct ip_mreq));

   socklen = sizeof(struct sockaddr_in);
 /*
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], "12345", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    } */
    int sockfd, numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], "12345", &hints, &servinfo)) != 0) {
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

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure


  int c = 0;
  while(1){
     // receive packet from socket
    status = recvfrom(sock, buffer, MAXBUFSIZE, 0, 
                     (struct sockaddr *)&saddr, &socklen);
    int fill = 0;
    int left = status;
    while (left){
        if(inspect(&buffer[status-left])){
            memcpy(&bufferc[fill], &buffer[status-left], 188);
            fill += 188;
        }
        left -= 188;
    }
//    printf("%d bytes received\n", status);
//    numbytes = send(sockfd, buffer, status, 0);
    numbytes = send(sockfd, bufferc, fill, 0);
//    printf("%d bytes sendt\n", numbytes);
/*    if (!(c++%10)) {
      printf(".");
      fflush(stdout); 
    } */
  }

   // shutdown socket
   shutdown(sock, 2);
   // close socket
   close(sock);

   return 0;
}
