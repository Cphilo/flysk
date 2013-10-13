#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define LEN 30
#define EXT_LEN 10
#define BUFFER_SIZE 10240

void err(const char *msg);
void handle_req(int sock);
void not_implemented(int sock);
void not_found(int sock);
void send_res(int sock, char *content, int stacode, char *stadsp, char *content_type);

char EXT[][LEN] = {"htm", "html", "xml", "txt", "css", "png", "gif", "jpg", "jpeg", "zip"};
char CON_TYPE[][LEN] = {"text/html", "text/html", "text/xml", "text/plain", "text/css", "image/png", "image/gif", "image/jpg", "image/jpeg", "application/zip"};

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

        /*double fork to avoid zoombie processes.*/
        if(!(pid=fork())) {
            if(!fork()) {
                handle_req(newsockfd);
            } else {
                exit(0);
            }
        } else {
            waitpid(pid, NULL, 0);
        }
    }
    close(sockfd);
    return 0;
}
void handle_req(int sock)
{
    int n, i, ext_index;
    char buffer[BUFFER_SIZE];
    char *http_me, *res, *req_file, *ext;
    int fp;
    bzero(buffer, BUFFER_SIZE);
    n = read(sock, buffer, BUFFER_SIZE);
    if(n<0)
        err("ERR reading from socket.");
    printf("Here is the message:%s\n", buffer);
    sscanf(buffer, "%s%s", http_me, req_file);
    printf("http method:%s\n", http_me);
    if(!strcmp(http_me, "GET") || !strcmp(http_me, "POST"))
    {
        //sscanf(buffer, "%s", req_file);
    } else {
        not_implemented(sock);
        return;
    }
    ext = index(req_file, '.');
    for(i=0, ext++, ext_index=-1;i<EXT_LEN;i++) {
        if(!strcmp(EXT[i], ext)) {
            ext_index=i;
            break;
        }
    }
    if(ext_index<0) {
        req_file = "index.html";
    }
    if(access(req_file, R_OK)!=-1) {
        fp = open(req_file, 'r');
        if(read(fp, res, BUFFER_SIZE)>0) {
            send_res(sock, res, 200, "OK", CON_TYPE[ext_index]);
            return;
        } 
    }
    not_found(sock);
}


void send_res(int sock, char *content, int stacode, char *stadsp, char *content_type)
{
    char *res;
    int n;
    sprintf(res, "HTTP/1.1 %d %s\r\nServer: Flysk Web Server\r\nContent-Length: %d\r\nConnection: close\r\nContent-Type: %s\r\n\r\n", stacode, stadsp, strlen(content), content_type);
    printf("response:%s\n", res);
    n = write(sock, res, strlen(res));
    if(n<0)err("ERR writing socket.");
    n = write(sock, content, strlen(content));
    if(n<0)err("ERR writing socket.");
    close(sock);
}

void not_implemented(int sock)
{
    send_res(sock, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"></head>\
            <body><h2>Flysk Web Server</h2>\
            <div>501 - Method Not Implemented</div></body></html>", 501, "Not Implemented", "text/html");
}

void not_found(int sock)
{
    send_res(sock, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"></head>\
            <body><h2>Flysk Web Server</h2>\
            <div>404 - Not Found</div></body></html>", 404, "Not Found", "text/html");
}
