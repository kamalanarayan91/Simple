/**
 * @file    httpparser.c
 * @authors Kamala Narayan B.S. (kamalanb)
 *          Srikanth Sedimbi (ssedimbi)
 * @date   Fri, 29 February 2015 
 *
 * @brief Routines that parse HTTP requests from the client and
 * frame the response for sending back to the client.
 *
 */

#include <httpparser.h>

/**
* parseRequest : parses the given http request and generates the
* appropriate response
* args: 
	buffer: incoming http request
	responseBuffer: buffer to fill the response
* return:
	none
*/
void parseRequest(char *buffer, bufStruct *response,char *rootDirPath) {

	int size = 0;
	int uriLength = 0;
	int httpLength = 0;
	char methodName[MAX_BUF_SIZE + 1];
	char uri[MAX_BUF_SIZE + 1];
	char httpVersion[MAX_BUF_SIZE + 1];
	char ch;
	FILE *fp = NULL;
	int method = -1;
	char resourcePath[MAX_PATH] = "";
	char finalURI[MAX_PATH]="";

	while((size < MAX_BUF_SIZE ) && ( (ch = buffer[size]) != ' ')) 
	{
		methodName[size] = ch;
		size++;
	}
	
	
	methodName[size] = '\0';
	
	//check Get method 
	response->entitySize = 0;
	
	if((method = checkMethod(methodName)) == FAILURE) 
	{
		serveError(501,response,FAILURE);
		return;
	}	


	/*increment size to skip space delimiter*/
	size++;
	
	/*check uri*/
	while((size < MAX_BUF_SIZE) && ((ch = buffer[size]) != ' ')) 
	{
		uri[uriLength] = ch;
		uriLength++;
		size++;

	}
	
	uri[uriLength] = '\0';
	

	getFinalURI(uri,finalURI);
	strcpy(resourcePath,rootDirPath);
	strcat(resourcePath,finalURI);

	//Check resource path:
	int fileError = checkFile(resourcePath);
	if(fileError != SUCCESS)
	{
		serveError(fileError,response,method);
		return;
	}

	//finding resource
	if((fp = openFile(resourcePath)) == NULL )
	{
		serveError(404,response,method);
		return;
	}


	/*increment size to skip space delimiter*/
    size++;

	while((size < MAX_BUF_SIZE ) && ( (ch = buffer[size]) != '\r')) 
	{
        httpVersion[httpLength] = ch;
		httpLength++;
        size++;
	}

	httpVersion[httpLength] = '\0';

	if(checkHttpVersion(httpVersion) == -1)
	{		
		serveError(505,response,method);
		return;
	}

	/*increment the size and check for '\n' */
	int returnVal = checkHeader(buffer,size);
	if(returnVal == FAILURE)
	{
		serveError(400,response,method);
		return;
	}


	/*Parse header if needed*/
	/*file return*/

	if(method == GET) 
	{
		serveGet(response,fp,resourcePath);
		return;
	}
	else if( method == HEAD) 
	{
		serveHead(response,fp,resourcePath);
		return;
	} 

	else 
	{
		/*Control  never reaches here*/
		exit(FAILURE);
	}

}


/**
*checkMethod: Checks if the given method is supported by Server.
*args : 
*	methodName: string with method name.
*return: 
*	-1 : method not found
*	 1 : Get Method
*	 2 : Head Method
*/
int checkMethod(char *methodName) 
{

	if(strcmp(methodName,"GET") == 0) 
	{
		
		return GET;
	}
	else if( strcmp(methodName,"HEAD") == 0) 
	{
		
		return HEAD;
	} 
	else 
	{
		
		return FAILURE;
	}
}


/**
*openFile: Checks if the given uri can be  served by Server.
*args : 
*       filePath: string with uri path.
*return: 
*       null : uri not found
*        fp : file pointer of the corresponding uri
*/
FILE *openFile(char *filePath) {
	
	FILE *fp = fopen(filePath,"r");
	return fp;
}



/**
*checkUri: Checks if the given uri can be  served by Server.
*args : 
*       uri: string with uri path.
*return: 
*       null : uri not found
*        fp : file pointer of the corresponding uri
*/
int checkFile(char *filePath) {
	
	//directory check
	struct stat buf;
	int status = stat(filePath,&buf);
	if(status == 0)
	{
		if(S_ISDIR(buf.st_mode))
			return NOT_PERMITTED;
	}

	int fd = open(filePath,O_RDONLY);
	if(fd > 0)
	{
		close(fd);
		return SUCCESS;
	}

	//error
	if(errno == EACCES)
	{
		return NOT_PERMITTED;
	}
	else if(errno == ENOENT)
	{
		return NOT_FOUND;
	}
	else
	{
		return SERVER_ERROR;
	}

	//wont' reach
	return FAILURE;
}



/**
*checkHttpVersion: Checks if the HTTP version is supported by Server.
*args : 
*       httpVersion: string with HTTP version.
*return: 
*       -1 : version not supported
*        0 : HTTP version is 1.0
*/
int checkHttpVersion(char *httpVersion) {

	if(strcmp(httpVersion,"HTTP/1.1") != 0) 
	{
		
		return FAILURE;
	} 
	else 
	{
		
	    return SUCCESS;
	}
}

/**
 * getFinalURI: validates the URI, makes changes to it
 * if necessary.
 * Assuming that the uri only starts with /
 * The final URI contains the result.
 */

void getFinalURI(char *uri,char *finalURI)
{
	//uri validation
	strcpy(finalURI,uri);

	//add index.html
	if(!strcmp(uri,"/"))
	{
	
		strcat(finalURI,boilerPlatePage);
	}

	return;	
	
}




/**
*serveGet : serves the client with the requested GET METHOD
* args:
*	response: response struct to be filled
*	fp : File pointer of the file to be sent
*return:
*	none
*/
void serveGet(bufStruct *response,FILE *fp,char *uri) {
	
	char *mimeBuf;
	char mimeHeader[MAX_PATH] ="Content-Type: ";

	

	//currently sending 200 OK
	// fill the file transfer
	strcpy(((response->buffer)+(response->bufSize)),response200);
	response->bufSize += strlen(response200);

	//Server
	strcpy(((response->buffer)+(response->bufSize)),server);
	response->bufSize += strlen(server);

	//Get content type
	mimeBuf = get_mime(uri);
	strcat(mimeHeader,mimeBuf);
	strcat(mimeHeader,"\r\n");
	strcpy(((response->buffer)+(response->bufSize)),mimeHeader);
	response->bufSize += strlen(mimeHeader);

	//Add Connection:close
	strcpy(((response->buffer)+(response->bufSize)),connectionClose);
	response->bufSize += strlen(connectionClose);
	
	//ending CRLF
	strcpy(((response->buffer)+(response->bufSize)),"\r\n");
	response->bufSize += strlen("\r\n");

	//Entity Body
	
	//Getting filesize
	fseek(fp,0,SEEK_END);
	response->entitySize = ftell(fp);
	rewind(fp);


	response->entityBuffer = malloc(response->entitySize);
	fread(response->entityBuffer,sizeof(char),response->entitySize,fp);

	//file is in buffer now
	fclose(fp);

	//end response
	return;
}


/**
*serveHead : serves the client with the requested HEAD METHOD
* args:
*       response: response struct to be filled
*       fp : File pointer of the file to be sent
*return:
*       none
*/
void serveHead(bufStruct *response, FILE *fp,char *uri) {
	char *mimeBuf;
	char mimeHeader[MAX_PATH] ="Content-Type: ";


	//currently sending 200 OK
	// fill the file transfer
	strcpy(((response->buffer)+(response->bufSize)),response200);
	response->bufSize += strlen(response200);

	//Server
	strcpy(((response->buffer)+(response->bufSize)),server);
	response->bufSize += strlen(server);

	//Get content type
	mimeBuf = get_mime(uri);
	strcat(mimeHeader,mimeBuf);
	strcat(mimeHeader,"\r\n");
	strcpy(((response->buffer)+(response->bufSize)),mimeHeader);
	response->bufSize += strlen(mimeHeader);

	//Add Connection:close
	strcpy(((response->buffer)+(response->bufSize)),connectionClose);
	response->bufSize += strlen(connectionClose);
	
	//ending CRLF
	strcpy(((response->buffer)+(response->bufSize)),"\r\n");
	response->bufSize += strlen("\r\n");
        
	//No- entity
    response->entitySize =0;

    fclose(fp);
    return;
}



/**
*serveError : Formulates the error response to be sent to the
*client.
*args:
*       errorCode: HTTP error Code
*       response: to be sent to the client
*       requestType: GET/HEAD/ OTHER
*return:
*       none
*/
void serveError(int errorCode, bufStruct *response,int requestType )
{
	char errorBody[MAX_PATH];

	switch(errorCode)
	{
		//Client Errors
		case 400: 	strcpy(((response->buffer)+(response->bufSize)),response400);
					response->bufSize += strlen(response404);
					strcpy(errorBody,"400: Bad Request\n");
					break;

		case 403:	strcpy(((response->buffer)+(response->bufSize)),response403);
					response->bufSize += strlen(response403);
					strcpy(errorBody,"403: Forbidden\n");
					break;

		case 404:	strcpy(((response->buffer)+(response->bufSize)),response404);
					response->bufSize += strlen(response404);
					strcpy(errorBody,"404: Not Found\n");
					break;

		//Server Errors
		case 500:	strcpy(((response->buffer)+(response->bufSize)),response500);
					response->bufSize += strlen(response500);
					strcpy(errorBody,"500: Internal Server Error\n");
					break;

		case 501:	strcpy(((response->buffer)+(response->bufSize)),response501);
					response->bufSize += strlen(response501);
					strcpy(errorBody,"501: Method Not Implemented\n");
					break;

		case 503:	strcpy(((response->buffer)+(response->bufSize)),response503);
					response->bufSize += strlen(response503);
					strcpy(errorBody,"503: Service Unavailable\n");
					break;

		case 505: 	strcpy(((response->buffer)+(response->bufSize)),response505);
					response->bufSize += strlen(response505);
					strcpy(errorBody,"505: HTTP Version Not Supported\n");
					break;


	};

	//Server
	strcpy(((response->buffer)+(response->bufSize)),server);
	response->bufSize += strlen(server);
		
	//Connection: Close
	strcpy(((response->buffer)+(response->bufSize)),connectionClose);
	response->bufSize += strlen(connectionClose);

	//CRLF
	strcpy(((response->buffer)+(response->bufSize)),"\r\n");
	response->bufSize += strlen("\r\n");
 

	//Entity Body
	if(requestType == GET)
	{
		//Not supposed to send if the request is HEAD
		response->entityBuffer = malloc(strlen(errorBody)+1);
		strcpy(response->entityBuffer,errorBody);
 		response->entitySize = strlen(errorBody)+1;
 	}
 	else
 	{
 		response->entitySize = 0;
 	}

    return;

}

/**
*checkHeader : checks the header fields, for Simple server,
*	       as these fields donot correspond to anything, we simply 
*	       read through the header and check if it is porperly terminated
*args:
*	buffer: pointer to request buffer
*	size :	position till where the buffer is read by parser
*
*return: 
*	0 : if header is proper.
*      -1 : if the header is malformed 
*/
int checkHeader(char *buffer,int size) {

	int ret = FAILURE;

	while(size < MAX_BUF_SIZE) {
		
		if(buffer[size] == '\r') {
			if((strncmp((buffer + size),"\r\n\r\n",4)) == 0) {
				ret = 0;
				break;
			}
		}
		size++;
	}
	
	return ret;
}
