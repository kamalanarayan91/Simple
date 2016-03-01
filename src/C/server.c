/**
 * @file   server.c
 * @authors Kamala Narayan B.S. (kamalanb)
 *          Srikanth Sedimbi (ssedimbi)
 * @date   Fri, 29 February 2015 
 *
 * @brief A simple web server that handles each client's 
 * requests using a seperate thread. The connection limit
 * for this server is set  at 25, after which the server
 * sends 503- Service Unavailable response to any client
 * that tries to connect to it.
 *
 */
/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
/* Includes related to socket programming */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

/*Includes for thread*/
#include<pthread.h>

/* Local includes from ./inc */
#include <log.h>
#include <helper.h>
#include <config.h>
#include <httpparser.h>

#define ARGS_NUM 2
#define MAX_LINE 4096
#define CONNECTION_LIMIT 25

static char path[MAX_PATH];

void *newClientThread(void *vargp);
static pthread_mutex_t connMutex;
static int globalConnectionCount = 0;


int main(int argc, char **argv)
{
    int port;
    int serv_sock;
    int optval = 1; 
    socklen_t len;
    struct sockaddr_in addr, client_addr;
    char *client_addr_string;
    DIR *rootDir;

    /*
     * ignore SIGPIPE, will be handled
     * by return value
     */
    signal(SIGPIPE, SIG_IGN);

    if (argc != (ARGS_NUM + 1)) {
        error_log("%s","Incorrect arguments provided\n"
                  "usage: ./server <port>");

        exit(EXIT_FAILURE);
    }

    client_addr_string = (char *) malloc(INET_ADDRSTRLEN);
    if (NULL == client_addr_string) {
        error_log("Unable to allocate memory for client_addr due to malloc() "
                  "error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Parse the port */
    port = atoi(argv[1]); 
    if ((port > MAX_PORT) || (port < MIN_PORT)) {
        error_log("%s", "Port must be in range 1024 to 65535");
        exit(EXIT_FAILURE);
    }


    /*Validate path*/
    strcpy(path,argv[2]);
    path[strlen(argv[2])+1]='\0';
    rootDir = opendir(path);
    if(rootDir==NULL)
    {
        printf("Please enter Valid directory path\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        closedir(rootDir);
    }

    /* Create a socket for listening */
    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error_log("socket() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    /*
     * bind() the socket to the ip address and the port. This actually
     * created the mapping between the socket and the IP:Port pair
     */
    if (bind(serv_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        error_log("bind() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int)) < 0) {
        error_log("setsockopt() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * listen() for incoming connections.
     * The server accepts SYN packets after this call succeeds. If a client
     * calls connect() before this, the OS kernel will response with a RST and
     * terminate the connection. The BACKLOG is the total amount of connections
     * that can be in ESTABLISHED state (SYN-SYN/ACK-ACK complete at the server
     * before accept() is called.
     */
    if (listen(serv_sock, BACKLOG) < 0) {
        error_log("listen() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    debug_log("Now listening on port %d", port);

    while(1) {
        
        /* Accept the client connection  */
        len = sizeof(client_addr);

        /* Get a socket to actually communicate withe client
         * The client_sock returned by accept() is used for further
         * communication with the client while the serv_sock is still used for
         * new connections. accept() blocks if no connections are present
         */
        
        int* client_sock = malloc(sizeof(int));

        *client_sock = accept(serv_sock,
                             (struct sockaddr *) &client_addr, &len);
        if (client_sock < 0) {
            error_log("Unable to add client due to accept() "
                      "error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        

        inet_ntop(AF_INET, &(client_addr.sin_addr),
                  client_addr_string, INET_ADDRSTRLEN);
        debug_log("Accepted connection from %s:%d",
                  client_addr_string, ntohs(client_addr.sin_port));


        pthread_t tid;
        pthread_create(&tid,NULL,newClientThread,client_sock);

        
    }

    return 0;
}

/**
*newClientThread : Services the client's request.
*args: client connection socket file descriptor.
*
*return: NULL
*/
void *newClientThread(void *vargp)
{
    pthread_detach(pthread_self());
    int client_sock = *((int*) vargp);
    free(vargp);
    bool connLimitFlag = false;
    pthread_mutex_lock(&connMutex);
    if(globalConnectionCount<CONNECTION_LIMIT)
    {
        globalConnectionCount++;
    }
    else
    {
        connLimitFlag=true;    
    }
    pthread_mutex_unlock(&connMutex);   
    int bytes_received, bytes_sent, total_sent;
    char buffer[MAX_LINE];
    int ret = 0;
    total_sent = bytes_received = bytes_sent = 0;

    /* Read the date sent from the client */
    
     if(!connLimitFlag) {
        while((( ret= recv(client_sock,(buffer + bytes_received), MAX_LINE - 1, 0)) > 0) && 
            (bytes_received < MAX_BUF_SIZE)) 
        {
          bytes_received += ret;
          if(strncmp((buffer+ (bytes_received -4)),"\r\n\r\n",4) == 0) {
               break;
          }
        }

        /*
         * Echo - Write (send) the data back to the client taking care of short
         * counts
         */
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
        
            bufStruct response;
            response.buffer = (char *) malloc(sizeof(char)*(MAX_BUF_SIZE + 1));
            response.bufSize = 0;
            response.entitySize=0;
            parseRequest(buffer,&response,path);
           
            while (total_sent != response.bufSize) 
            {
                bytes_sent = send(client_sock, response.buffer + total_sent,
                                  response.bufSize - total_sent, 0);
                if (bytes_sent <= 0) {
                    break;
                } else {
                    total_sent += bytes_sent;
                }
            }

            //reset for file transfer
            total_sent = 0;

            //Send file if applicable
            while (total_sent != response.entitySize) {
                bytes_sent = send(client_sock, response.entityBuffer + total_sent,
                                  response.entitySize - total_sent, 0);
                if (bytes_sent <= 0) {
                    break;
                } else {
                    total_sent += bytes_sent;
                }

            }


            if(response.entitySize !=0)
            {
                free(response.entityBuffer);
            }

            free(response.buffer);

        }

    }
    else
    {
        //Send 503- Service Unavailable.
        bufStruct response;
        response.buffer = (char *) malloc(sizeof(char)*(MAX_BUF_SIZE + 1));
        response.bufSize = 0;
        response.entitySize=0;

        serveError(503,&response,FAILURE);
        while (total_sent != response.bufSize) 
        {
            bytes_sent = send(client_sock, response.buffer + total_sent,
                              response.bufSize - total_sent, 0);
            if (bytes_sent <= 0) {
                break;
            } else {
                total_sent += bytes_sent;
            }
        }

        if(response.entitySize !=0)
        {
            free(response.entityBuffer);
        }
        free(response.buffer);
            
        
    }

    debug_log("Closing connection %s:%d",
              client_addr_string, ntohs(client_addr.sin_port));
    /* Our work here is done. Close the connection to the client */
    close(client_sock);
    pthread_mutex_lock(&connMutex);
    if(globalConnectionCount>0)
    {
        globalConnectionCount--;
    }
    pthread_mutex_unlock(&connMutex);

    return NULL;
}
