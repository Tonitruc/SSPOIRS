#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include "../logger.h"

#define LOG_FILE "client_log.txt"

// TCP
// command: UPLOAD, DOWNLOAD, CONNECT, DISCONNECT
// detect connection lost, request to reconnect
// received data pecsantage
// log file

void run();
void start_client(int* cfd, const char* serverName);
void upload(int cfd, const char* file);
void download(int cfd, const char* file);