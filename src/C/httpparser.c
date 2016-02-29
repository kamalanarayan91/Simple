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
void parseRequest(char *buffer, bufStruct *response) {

	int size = 0;
	int uriLength = 0;
	int httpLength = 0;
	char methodName[MAX_BUF_SIZE + 1];
	char uri[MAX_BUF_SIZE + 1];
	char httpVersion[MAX_BUF_SIZE + 1];
	char ch;
	FILE *fp = NULL;
	int method = -1;
	while((size < MAX_BUF_SIZE ) && ( (ch = buffer[size]) != ' ')) {
		methodName[size] = ch;
		size++;
	}
	
	methodName[size] = '\0';
	printf("Method = %s\n",methodName);
	//check Get method 
	
	printf("Return value =  %d \n",checkMethod(methodName));
	
	if((method = checkMethod(methodName)) == -1) {

		strcpy(((response->buffer)+(response->bufSize)),response501);
		response->bufSize += strlen(response501);
		strcpy(((response->buffer)+(response->bufSize)),server);
		response->bufSize += strlen(server);
		printf("Response buf = %s\n",response->buffer);
		return;
	}	


	/*increment size to skip space delimiter*/
	size++;
	
	/*check uri*/
	while((size < MAX_BUF_SIZE) && ((ch = buffer[size]) != ' ')) {
		uri[uriLength] = ch;
		uriLength++;
		size++;

	}
	uri[uriLength] = '\0';
	printf("Uri received = %s \n",uri);
	checkUri(uri);
	
	if((fp = checkUri(uri)) == NULL ) {
		strcpy(((response->buffer)+(response->bufSize)),response404);
		response->bufSize += strlen(response404);
		strcpy(((response->buffer)+(response->bufSize)),server);
		response->bufSize += strlen(server);
		printf("Response buf = %s\n",response->buffer);
		return;

	}	

	/*increment size to skip space delimiter*/
        size++;

	while((size < MAX_BUF_SIZE ) && ( (ch = buffer[size]) != '\r')) {
                httpVersion[httpLength] = ch;
		httpLength++;
                size++;
        }

	httpVersion[httpLength] = '\0';

	if(checkHttpVersion(httpVersion) == -1) {
	
		strcpy(((response->buffer)+(response->bufSize)),response505);
		response->bufSize += strlen(response505);
		strcpy(((response->buffer)+(response->bufSize)),server);
		response->bufSize += strlen(server);
		printf("Response buf = %s\n",response->buffer);
		return;


	}

	/*increment the size and check for '\n' */
	size++;
	if(buffer[size] != '\n') {

		printf("Malformed request \n");
		//return bad req

	}

	/*Parse header if needed*/
	/*file return*/

	if(method == 1) {
		serveGet(response,fp);
		return;
	} else if( method == 2) {

		serveHead(response,fp);
	 } else {
		printf("Why am I here?\n");
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
int checkMethod(char *methodName) {

	if(strcmp(methodName,"GET") == 0) {
		printf("GET Method found \n");
		return 1;
	} else if( strcmp(methodName,"HEAD") == 0) {
		printf("HEAD method found \n");
		return 2;
	} else {
		printf("Method not found \n");
		return -1;
	}
}


/**
*checkUri: Checks if the given uri can be  served by Server.
*args : 
*       uri: string with uri path.
*return: 
*       null : uri not found
*        fp : file pointer of the corresponding uri
*/
FILE *checkUri(char *uri) {
	
	FILE *fp = fopen(uri,"r");
	return fp;
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

	if(strcmp(httpVersion,"HTTP/1.0") != 0) {
		printf("Version not supported \n");
		return -1;
	} else {
		printf("Supports HTTP %s\n", httpVersion);
	        return 0;

	}
} 

/**
*serveGet : serves the client with the requested GET METHOD
* args:
*	response: response struct to be filled
*	fp : File pointer of the file to be sent
*return:
*	none
*/
void serveGet(bufStruct *response,FILE *fp) {
	//currently sending 200 OK
	// fill the file transfer
	strcpy(((response->buffer)+(response->bufSize)),response200);
	response->bufSize += strlen(response200);
	strcpy(((response->buffer)+(response->bufSize)),server);
	response->bufSize += strlen(server);
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
void serveHead(bufStruct *response, FILE *fp) {
        //currently sending 200 OK
        // fill the head response
        strcpy(((response->buffer)+(response->bufSize)),response200);
        response->bufSize += strlen(response200);
        strcpy(((response->buffer)+(response->bufSize)),server);
        response->bufSize += strlen(server);
        return;
}

