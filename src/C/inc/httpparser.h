#ifndef _HTTP_PARSER_
#define _HTTP_PARSER_

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<helper.h>
#define MAX_BUF_SIZE 1024
#define MAX_PATH 1024
#define SUCCESS 1
#define FAILURE 0


static const char response200[] = "HTTP/1.0 200 OK\r\n";
static const char response400[] = "HTTP/1.0 400 Bad Request\r\n";
static const char response404[] = "HTTP/1.0 404 Not Found\r\n";
static const char response501[] = "HTTP/1.0 501 Not Implented\r\n";
static const char response505[] = "HTTP/1.0 505 HTTP Version not Supported\r\n";
static const char server[] = "Server: Simple/1.0\r\n";
static const char boilerPlatePage[] = "index.html";
static const char contentLength[] = "index.html";
static const char connectionClose[] = "Connection: Close\r\n";

typedef struct bufStruct{
	char *buffer;
	int bufSize;
	char *fileBuffer;
	size_t fileSize;
}bufStruct;


void parseRequest(char *buffer, bufStruct *response,char *rootDirPath);
int checkMethod(char *methodName);
FILE *checkUri(char *uri);
int checkHttpVersion(char *httpVersion);
void serveGet(bufStruct *response,FILE *fp,char *uri);
void serveHead(bufStruct *response,FILE *fp);
void getFinalURI(char *uri,char *finalURI);
#endif
