// Multicast catcher
// Change: IP_MULTICAST_LOOP : Enable / Disable loopback for outgoing messages
// 
// Compile : gcc -o client client.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFSIZE 65536 // Max UDP Packet size is 64 Kbyte

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
   int sock, status, socklen;
   char buffer[MAXBUFSIZE];
   struct sockaddr_in saddr;
   struct in_addr iaddr;
   unsigned char ttl = 3;
   unsigned char one = 1;

   // set content of struct saddr and imreq to zero
   memset(&saddr, 0, sizeof(struct sockaddr_in));
   memset(&iaddr, 0, sizeof(struct in_addr));

   // open a UDP socket
   sock = socket(PF_INET, SOCK_DGRAM, 0);
   if ( sock < 0 )
     perror("Error creating socket"), exit(0);

   saddr.sin_family = PF_INET;
   saddr.sin_port = htons(0); // Use the first free port
//   saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface
   inet_pton(AF_INET, "10.0.10.1", &saddr.sin_addr.s_addr);
   status = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));

   if ( status < 0 )
     perror("Error binding socket to interface"), exit(0);

   iaddr.s_addr = inet_addr("10.0.10.1"); // use DEFAULT interface

   // Set the outgoing interface to DEFAULT
   setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
              sizeof(struct in_addr));

   // Set multicast packet TTL to 3; default TTL is 1
   setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
              sizeof(unsigned char));

   // send multicast traffic to myself too
   status = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                       &one, sizeof(unsigned char));

   // set destination multicast address
   saddr.sin_family = PF_INET;
//   saddr.sin_addr.s_addr = inet_addr("226.0.0.1");
   saddr.sin_addr.s_addr = inet_addr("10.0.10.51");
   saddr.sin_port = htons(5500);


   socklen = sizeof(struct sockaddr_in);
   socklen_t addr_len;
   int numbytes;

   /* int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, "12345", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
    */
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
//    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, "12345", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    printf("binding..\n");
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

/*    printf("listen..\n");
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(1);
    }
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        //continue;
    } */

    printf("listener: waiting to recvfrom...\n");

	int c = 0; 
	while(1){
		if ((numbytes = recvfrom(sockfd, buffer, MAXBUFSIZE-1 , 0,
        		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		        perror("recvfrom");
	        	exit(1);
		}

//		printf("listener: got packet from %s\n",
//			inet_ntop(their_addr.ss_family,
//			get_in_addr((struct sockaddr *)&their_addr),
//			s, sizeof s));
//		printf("listener: packet is %d bytes long\n", numbytes);
   		status = sendto(sock, buffer, numbytes, 0,
                     (struct sockaddr *)&saddr, socklen);
		if(!(c++%10)){
			printf(".");
			fflush(stdout);
		}
//		printf("sendt %d bytes\n", status);
	}


   // shutdown socket
   shutdown(sock, 2);
   // close socket
   close(sock);

   return 0;
}
