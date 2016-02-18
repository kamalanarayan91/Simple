#!/usr/bin/env python

# simple.py - Prototype implementation of Simple for 14740 Project 2
#
# Author - Chinmay Kamat <chinmaykamat@cmu.edu>
#
# Last Modified : Thu, 14 February 2013 20:08:12 EST

import os
import sys
import BaseHTTPServer

# hard-coded MIME types
MIME = {
    '.html': 'text/html',
    '.css': 'text/css',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.gif': 'image/gif'
}

# Server configuration
PORT = 8080
WWW = os.getcwd() + 'www'


# the class for the Simple HTTP server
class SimplePrototype(BaseHTTPServer.HTTPServer):
    pass


# Main class to handle all the requests
class SimpleHandlerPrototype(BaseHTTPServer.BaseHTTPRequestHandler):
    # Error handling functions
    def return404(self):
        self.send_error(404, 'Not Found')

    def return500(self):
        self.send_error(500, 'Not Found')

    def return505(self):
        self.send_error(505, 'HTTP Version not supported')

    # Make sure that the HTTP version in request is either 1.0 or 1.1
    def check_HTTP_version(self):
        version = self.request_version.split('/')
        version = version[1].split('.')
        if version[0] != '1' or (version[1] != '0' and version[1] != '1'):
            self.return505()
            return None
        else:
            return version

     # Function to handle GET requests
    def do_GET(self):
        # Check HTTP version
        if self.check_HTTP_version() is None:
            return
        # Construct the complete path of the requested URI
        URI = self.path
        full_path = os.path.normpath(WWW + URI)

        # If URI == '/' user entered something like http://127.0.0.1:8080/ in
        # browser. Return index.html to the user
        if os.path.isdir(full_path):
            full_path = os.path.join(full_path, 'index.html')

        # Check that the complete path actually exists. Send 404 if not
        if not os.path.exists(full_path):
            self.return404()
            return

        # Init the response body
        response_body = ''

        # Get the extension of the file to find mime type
        basename, extension = os.path.splitext(full_path)

        # read in full file
        try:
            with open(full_path, 'r') as f:
                response_body = f.read()
        except IOError:
            self.return_500()
            return

        # get proper mimetype
        try:
            mimetype = MIME[extension.lower()]
        except KeyError:
            # Default mime type
            mimetype = 'application/octet-stream'

        # send response; Server header is auto-generated
        self.send_response(200, 'OK')
        # HTTP/1.0 response close connection after sending response
        self.send_header('Connection', 'close')
        self.send_header('Content-Type', mimetype)
        self.send_header('Content-Length', len(response_body))
        self.end_headers()
        # Send the reponse to the client
        self.wfile.write(response_body)
        # Close the connection - HTTP/1.0
        self.close_connection = 1

    # Function to handle GET requests
    def do_HEAD(self):
        # Check HTTP version
        if self.check_HTTP_version() is None:
            return
        # Construct the complete path of the requested URI
        URI = self.path
        full_path = os.path.normpath(WWW + URI)

        # If URI == '/' user entered something like http://127.0.0.1:8080/ in
        # browser. Return index.html to the user
        if os.path.isdir(full_path):
            full_path = os.path.join(full_path, 'index.html')

        # Check that the complete path actually exists. Send 404 if not
        if not os.path.exists(full_path):
            self.return404()
            return

        # Get the extension of the file to find mime type
        basename, extension = os.path.splitext(full_path)

        # get stats - for Content-Length
        fsize = os.path.getsize(full_path)

        # get proper mimetype
        try:
            mimetype = MIME[extension.lower()]
        except KeyError:
            mimetype = 'application/octet-stream'

        # send response; Server header auto-generated
        self.send_response(200, 'OK')
        self.send_header('Connection', 'close')
        self.send_header('Content-Type', mimetype)
        self.send_header('Content-Length', fsize)
        self.end_headers()
        self.close_connection = 1

# Since we do not define methods for POST, PUT etc, the BasicHTTPServer class
# returns a HTTP/1.0 501 Unsupported method for those requests. You will need to
# add this to your implementation


# We initialize Server here. Server starts listening for connection at the end
# of this function
def run():
    handler = SimpleHandlerPrototype

    # set version strings for auto-generated Server header and response
    handler.server_version = 'Simple/1.0'
    handler.sys_version = ''
    handler.protocol_version = 'HTTP/1.0'

    # server start up
    server_address = ('0.0.0.0', PORT)
    httpd = SimplePrototype(server_address, handler)
    # Listen on PORT forever
    httpd.serve_forever()


# This is where the program begins execution
if __name__ == '__main__':
    # Make sure all the arguments are correct
    if len(sys.argv) < 3:
        sys.stderr.write('Usage: %s <port> <Absolute path to www directory>\n'
                         % (sys.argv[0]))
        sys.exit(1)
    else:
        try:
            PORT = int(sys.argv[1])
        except ValueError:
            sys.stderr.write('Usage: %s <port> <Absolute path to www directory>\n'
                             % (sys.argv[0]))
            sys.stderr.write('Port number should be with 1024 and 65535\n')
            sys.exit(1)
        WWW = sys.argv[2]

    if (PORT > 65535 or PORT < 1024):
        sys.stderr.write('Usage: %s <port> <Absolute path to www directory>\n'
                         % (sys.argv[0]))
        sys.stderr.write('Port number should be with 1024 and 65535\n')
        sys.exit(1)
    elif not os.path.isdir(WWW):
        sys.stderr.write('Usage: %s <port> <Absolute path to www directory>\n'
                         % (sys.argv[0]))
        sys.stderr.write('"%s" does not exist or is not a directory\n'
                         % WWW)
        sys.exit(1)
    # Start the server
    run()
