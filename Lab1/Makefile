CC = gcc

.PHONY: clean

all: server client
server: Server/server.c Server/main.c Server/server.h logger.h logger.c Makefile
		$(CC) Server/server.c Server/main.c logger.c -o server -lcurl -lmpg123 -lasound
client: Client/client.c Client/main.c Client/client.h logger.h logger.c Makefile
		$(CC) Client/client.c Client/main.c logger.c -o client
clean:
		rm server client