#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

void err(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    if(argc < 3)
    {
        fprintf(stderr, "usage %s hostname port", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        err("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if(server == NULL)
    {
        fprintf(stderr, "ERROR, no such host");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
        err("ERROR connecting");
    printf("Please enter the message:");
    bzero(buffer, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);
    //buffer = "GET / ";
    n = write(sockfd, buffer, strlen(buffer));
    if(n<0)
        err("ERROR writing to socket.");
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE);
    if(n<0)
        err("ERROR reading from socket");
    printf("%s", buffer);
    close(sockfd);
    return 0;
}


