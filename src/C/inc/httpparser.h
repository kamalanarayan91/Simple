#ifndef _HTTP_PARSER_
#define _HTTP_PARSER_

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#define MAX_BUF_SIZE 1024


static const char response200[] = "HTTP/1.0 200 OK\r\n";
static const char response400[] = "HTTP/1.0 400 Bad Request\r\n";
static const char response404[] = "HTTP/1.0 404 Not Found\r\n";
static const char response501[] = "HTTP/1.0 501 Not Implented\r\n";
static const char response505[] = "HTTP/1.0 505 HTTP Version not Supported\r\n";
static const char server[] = "Server: Simple/1.0\r\n";

typedef struct bufStruct{
	char *buffer;
	int bufSize;
}bufStruct;


void parseRequest(char *buffer, bufStruct *response);
int checkMethod(char *methodName);
FILE *checkUri(char *uri);
int checkHttpVersion(char *httpVersion);
void serveGet(bufStruct *response,FILE *fp);
void serveHead(bufStruct *response,FILE *fp);
#endif
