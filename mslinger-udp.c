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

int main(int argc, char *argv[])
{
   int sock, status, socklen;
   char buffer[MAXBUFSIZE];
   struct sockaddr_in saddr;
   struct ip_mreq imreq;

   // set content of struct saddr and imreq to zero
   memset(&saddr, 0, sizeof(struct sockaddr_in));
   memset(&imreq, 0, sizeof(struct ip_mreq));

   // open a UDP socket
   sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
   if ( sock < 0 )
     perror("Error creating socket"), exit(0);

   int buffsize = 12582912;
    status = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(int));
    if (status < 0)
       perror("setsockopt");
    status = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(int));
    if (status < 0)
       perror("setsockopt");

   saddr.sin_family = PF_INET;
   saddr.sin_port = htons(atoi(argv[3])); // listen on port 4096
   saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface
   status = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));

   if ( status < 0 )
     perror("Error binding socket to interface"), exit(0);

   imreq.imr_multiaddr.s_addr = inet_addr(argv[2]);
   //imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface
   imreq.imr_interface.s_addr = inet_addr("129.240.203.149"); // use DEFAULT interface

   // JOIN multicast group on default interface
   status = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
              (const void *)&imreq, sizeof(struct ip_mreq));

   socklen = sizeof(struct sockaddr_in);

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
    status = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(int));
    if (status < 0)
       perror("setsockopt");
    status = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(int));
    if (status < 0)
       perror("setsockopt");

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
  int c = 0;
  while(1){
     // receive packet from socket
    status = recvfrom(sock, buffer, MAXBUFSIZE, 0, 
                     (struct sockaddr *)&saddr, &socklen);
//    printf("%d bytes received\n", status);
    numbytes = sendto(sockfd, buffer, status, 0, p->ai_addr, p->ai_addrlen);
//    printf("%d bytes sendt\n", numbytes);
    if (!(c++%10)) {
      printf(".");
      fflush(stdout);
    }
  }

   // shutdown socket
   shutdown(sock, 2);
   // close socket
   close(sock);

   return 0;
}
