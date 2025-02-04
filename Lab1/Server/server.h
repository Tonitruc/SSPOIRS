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

#define BUFFER_SIZE 4096

#define LOG_FILE "server_log.txt"

// while send generate persantage of transmitted data
// detect lost connection/disconnection
// SO_KEEPALIVE param
// log file
// make cache for broken transmittions
// send speed (or client?)

void run();
void start_server();
void process_client(int cfd);
void receive_data(int cfd, const char* file);
void send_data(int cfd, const char* file);
void echo();
void server_time();
void mhif_command();
void play_mp3(const char* filename);