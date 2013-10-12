#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void err(const char *msg)
{
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, n, pid;
    socklen_t clien;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    if(argc < 2)
    {
        fprintf(stderr, "ERR, no port provided");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        err("ERR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        err("ERR on binding");
    listen(sockfd, 5);
    clien = sizeof(cli_addr);
    while(1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clien);
        if(newsockfd<0)
            err("ERROR on accept.");
        pid = safe_fork();
        if(!pid) {
            exit(0);
            close(newsockfd);
        }
    }
    close(sockfd);
    return 0;
}
void dostuff(int sock)
{
    int n;
    char buffer[256];
    bzero(buffer, 256);
    n = read(sock, buffer, 255);
    if(n<0)
        err("ERR reading from socket.");
    printf("Here is the message:%s\n", buffer);
    n = write(sock, "I got yur message", 18);
    if(n<0)err("ERR writing to socket.");
}

//double fork to avoid zobie processes.
int safe_fork()
{
    int pid;
    if (!pid=fork()) 
    {
        if (!fork()) {
            /* this is the child that keeps going */
            dostuff(); /* or exec */
        } else {
            /* the first child process exits */
            exit(0);
        }
    } else {
        /* this is the original process */  
        /* wait for the first child to exit which it will immediately */
        waitpid(pid);
    }
    return pid;
}
