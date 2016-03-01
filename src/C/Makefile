CC      = gcc
CFLAGS  = -Wall -Werror -I ./inc -pthread

#default: httpparser getmime server client
default: getmime server client

.PHONY: clean

#httpparser: httpparser.c
getmime: getmime.c helper.c
server: server.c httpparser.c helper.c
client: client.c

debug: CFLAGS =  -pthread -g  -Wall -Werror -DDEBUG -DLOG_LEVEL=2  -I ./inc
debug: getmime server client

clean:
	rm -f *.o getmime server client
