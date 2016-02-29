#ifndef _HTTP_PARSER_
#define _HTTP_PARSER_

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<helper.h>
#include<sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define SERVER_ERROR 505
#define NOT_FOUND 404
#define NOT_PERMITTED 403

#define MAX_BUF_SIZE 1024
#define MAX_PATH 1024
#define SUCCESS 1
#define FAILURE -1
#define GET 1
#define HEAD 2


static const char response200[] = "HTTP/1.0 200 OK\r\n";

static const char response400[] = "HTTP/1.0 400 Bad Request\r\n";
static const char response403[] = "HTTP/1.0 403 Forbidden\r\n";
static const char response404[] = "HTTP/1.0 404 Not Found\r\n";

static const char response500[] = "HTTP/1.0 500 Internal Server Error\r\n";
static const char response501[] = "HTTP/1.0 501 Not Implemented\r\n";
static const char response503[] = "HTTP/1.0 503 Service Unavailable\r\n";
static const char response505[] = "HTTP/1.0 505 HTTP Version not Supported\r\n";

static const char server[] = "Server: Simple/1.0\r\n";
static const char boilerPlatePage[] = "index.html";

static const char connectionClose[] = "Connection: close\r\n";

typedef struct bufStruct{
	char *buffer;
	int bufSize;
	char *entityBuffer;
	size_t entitySize;
}bufStruct;


void parseRequest(char *buffer, bufStruct *response,char *rootDirPath);
int checkMethod(char *methodName);
FILE *openFile(char *uri);
int checkHttpVersion(char *httpVersion);
void serveGet(bufStruct *response,FILE *fp,char *uri);
void serveHead(bufStruct *response,FILE *fp,char *uri);
void getFinalURI(char *uri,char *finalURI);
void serveError(int errorCode, bufStruct *response,int requestType );
int checkFile(char *path);
int checkHeader(char *buffer,int size);
#endif

