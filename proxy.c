/*
 * Starter code for proxy lab.
 * Feel free to modify this code in whatever way you wish.
 */

/* Some useful includes to help you get started */

#include "csapp.h"
#include "http_parser.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

/*
 * Debug macros, which can be enabled by adding -DDEBUG in the Makefile
 * Use these if you find them useful, or delete them if not
 */
#ifdef DEBUG
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_printf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dbg_assert(...)
#define dbg_printf(...)
#endif

/*
 * Max cache and object sizes
 * You might want to move these to the file containing your cache implementation
 */
#define MAX_CACHE_SIZE (1024 * 1024)
#define MAX_OBJECT_SIZE (100 * 1024)

/*
 * String to use for the User-Agent header.
 * Don't forget to terminate with \r\n
 */
static const char *header_user_agent = "User-Agent: Mozilla/5.0"
                                       " (X11; Linux x86_64; rv:3.10.0)"
                                       " Gecko/20191101 Firefox/63.0.1\r\n";



void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg) {
    char buf[MAXLINE];
    char body[MAXBUF];
    size_t buflen;
    size_t bodylen;
    bodylen = snprintf(body, MAXBUF,
                       "<html>\r\n"
                       "<head><title>Tiny Error</title></head>\r\n"
                       "<body bgcolor=\"ffffff\">\r\n"
                       "<h1>%s: %s</h1>\r\n"
                       "<p>%s: %s</p>\r\n"
                       "<hr><em>The Tiny Web server</em>\r\n"
                       "</body></html>\r\n",
                       errnum, shortmsg, longmsg, cause);
    if (bodylen >= MAXBUF) {
        return; // Overflow!
    }

    /* Build the HTTP response headers */
    buflen = snprintf(buf, MAXLINE,
                      "HTTP/1.0 %s %s\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: %zu\r\n\r\n",
                      errnum, shortmsg, bodylen);
    if (buflen >= MAXLINE) {
        return; // Overflow!
    }

    /* Write the headers */
    if (rio_writen(fd, buf, buflen) < 0) {
        fprintf(stderr, "Error writing error response headers to client\n");
        return;
    }

    /* Write the body */
    if (rio_writen(fd, body, bodylen) < 0) {
        fprintf(stderr, "Error writing error response body to client\n");
        return;
    }
}

void errorHandler(parser_t *parser,char buf[], int fd, char* method, char* version)
{
    parser_state state = parser_parse_line(parser, buf);
    if (state == ERROR) {
        clienterror(fd, buf, "400", "Bad Request",
                    "Tiny could not handle this request (ERROR)");
        return;
    }
    parser_retrieve(parser, METHOD, &method);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, buf, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }

    parser_retrieve(parser, HTTP_VERSION, &version);
    if (strcasecmp(version, "1.0") && strcasecmp(version, "1.1")) {
        clienterror(fd, buf, "400", "Bad Request",
                    "Tiny could not handle this request (HTTP_VERSION)");
        return;
    }
}
bool connectionHelper(int sfd) {
    if (sfd >= 0) {
        return true;
    }
    fprintf(stderr, "Connection failed\n");
    return false;
}
void transferHelper(rio_t curr, char buf[], int fd)
{
    for (ssize_t i = rio_readnb(&curr, buf, MAXLINE); i>0; i = rio_readnb(&curr, buf, MAXLINE))
    {
        rio_writen(fd, buf, i);
    }
}
void doit(int fd, char* mID, char* vID, char* hostID, char* portID, char* path) {
    int serverfd;
    char buf[MAXLINE];
    char request[MAXLINE];
    char line[MAXLINE];
    char host[MAXLINE];
    char backup[MAXLINE];
    rio_t rioOne;
    rio_t rioTwo;
    parser_t *currParser;

    rio_readinitb(&rioOne, fd);
    rio_readlineb(&rioOne, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);


    currParser = parser_new();
    errorHandler(currParser, buf, fd, mID, vID);
    parser_retrieve(currParser, HOST, &hostID);
    parser_retrieve(currParser, PORT, &portID);
    parser_retrieve(currParser, PATH, &path);
    serverfd = open_clientfd(hostID, portID);
    if(!connectionHelper(serverfd))
    {
        return;
    }

    sprintf(line, "GET %s HTTP/1.0\r\n", path);
    sprintf(host, "Host: ");
    sprintf(host + strlen(host), hostID);
    sprintf(host + strlen(host), ":");
    sprintf(host + strlen(host), portID);
    sprintf(host + strlen(host), "\r\n");

    
    for (int i = rio_readlineb(&rioOne, buf, MAXLINE); i>0; i =rio_readlineb(&rioOne, buf, MAXLINE))
    {  
        if (strcmp(buf, "\r\n")!=0) {
            if (strncasecmp(buf, "Host", 4)==0) {
                sprintf(host, buf);
                continue;
            }
            else if (strncasecmp(buf, "User-Agent", 10) &&
            strncasecmp(buf, "Connection", 10) &&
            strncasecmp(buf, "Proxy-Connection", 16)) {
                strcat(backup, buf);
            }
            
        }
        else{
            break;
        }
    }
    sprintf(request, line);
    sprintf(request + strlen(request), host);
    sprintf(request + strlen(request), header_user_agent);
    sprintf(request + strlen(request), "Connection: close\r\n");
    sprintf(request + strlen(request), "Proxy-Connection: close\r\n");
    sprintf(request + strlen(request), backup);
    sprintf(request + strlen(request), "\r\n");

    
    rio_readinitb(&rioTwo, serverfd);
    rio_writen(serverfd, request, strlen(request));
    transferHelper(rioTwo,buf,fd);
    parser_free(currParser);
    close(serverfd);
    return;
}
void *thread(void *p) {
    const char *method;
    const char *version;
    const char *host;
    const char *port;
    const char *path;
    int *intp = (int*)p;
    int connfd = *(intp);
    pthread_detach(pthread_self());
    free(p);
    doit(connfd,method,version,host,port,path);
    close(connfd);
    return NULL;
}

int main(int argc, char **argv) {
    int listenfd;
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char host[MAXLINE];
    char port[MAXLINE];
    pthread_t tid;
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);
    listenfd = open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (*connfdp < 0) {
            perror("accept error");
            continue;
        }
        int temp = getnameinfo((struct sockaddr *)&clientaddr, clientlen, host, MAXLINE,
                    port, MAXLINE, 0);
        if(temp ==0)
        {
            printf("Accepted connection from (%s, %s)\n", host, port);
            pthread_create(&tid, NULL, thread, connfdp);
        }
        
    }

    return 0;
}

