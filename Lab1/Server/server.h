

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

#include <alsa/asoundlib.h>
#include <curl/curl.h>
#include <mpg123.h>

#include "../settings.h"

#define BUFFER_SIZE 4096

#define LOG_FILE "server_log.txt"

// while send generate persantage of transmitted data
// detect lost connection/disconnection
// SO_KEEPALIVE param
// log file
// make cache for broken transmittions
// send speed (or client?)

extern SETTINGS* settings;

void run();
void start_server(int *sfd);
void process_client(int cfd);
void receive_data(int cfd, const char* filePath);
void send_data(int cfd, char* filePath);
void echo();
void server_time();
void settings_command(char* command);