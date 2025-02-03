#include "client.h"

void run(const char* server) {
    
    log_message(LOG_FILE, LOG_INFO, "Start client work");

    char command[80];
    int cfd;
    start_client(&cfd, server);

    while (1) {
        
        fgets(command, sizeof(command), stdin);
        if (strcmp(command, "UPLOAD") == 0) {
            //char* file = command + 7;
            upload(cfd, command);
        }
        else if (strcmp(command, "DOWNLOAD") == 0) {
            //char* file = command + 9;
            download(cfd, command);
        }
        else if (strcmp(command, "QUIT") == 0)
            break;
    }

    log_message(LOG_FILE, LOG_INFO, "Stop client work");
    close(cfd);
}

void start_client(int* cfd, const char* serverName) {

    struct hostent* server;
    struct sockaddr_in addr;
    
    server = gethostbyname(serverName);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    bcopy((char*)server->h_addr_list[0], (char*)&addr.sin_addr.s_addr, server->h_length);

    *cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*cfd == -1) {
        log_message(LOG_FILE, LOG_CRITICAL, "Can't open socket");
        close(*cfd);
        exit(errno);
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(*cfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (connect(*cfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        if (errno == ENOENT) {
            sleep(1);
            continue;
        } else {
            log_message(LOG_FILE, LOG_CRITICAL, "Server connection error");
            close(*cfd);
            exit(errno);
        }
    }
}

void upload(int cfd, const char* command) {

    log_message(LOG_FILE, LOG_INFO, "Start upload file to server");
    unsigned char buffer[80];
    int fileSize = -1, sent = 0, bytesRead;
    const char* file = command + 7;

    FILE* f = fopen(file, "rb");
    if (f == NULL) {
        log_message(LOG_FILE, LOG_CRITICAL, "Can't open file to send data to server");
        fclose(f);
        return;
    }

    write(cfd, command, sizeof(command));
    if (read(cfd, &fileSize, sizeof(fileSize)) >= 0 && fileSize == -1) {
        log_message(LOG_FILE, LOG_CRITICAL, "Error while create file on server");
        fclose(f);
        return;
    }

    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    rewind(f);
    write(cfd, &fileSize, sizeof(fileSize));

    while (sent < fileSize && (sent += fread(buffer, 1, sizeof(buffer), f))) {
        write(cfd, buffer, sizeof(buffer));
        // display persantage and amount of sent bytes
    }

    log_message(LOG_FILE, LOG_INFO, "Data successfully sent to server");
    fclose(f);
}

void download(int cfd, const char* command) {

    log_message(LOG_FILE, LOG_INFO, "Start download data from server");
    // process lost connection
    unsigned char buffer[80];
    int fileSize = -1, received = 0;
    const char* file = command + 9;

    // send command to server && check that file exists
    write(cfd, command, sizeof(command));
    if (read(cfd, &fileSize, sizeof(fileSize)) >= 0 && fileSize == -1) {
        log_message(LOG_FILE, LOG_ERROR, "No such file on server");
        return;
    }

    FILE* f = fopen(file, "wb");
    if (f == NULL) {
        log_message(LOG_FILE, LOG_ERROR, "Can't create file to receive data from server");
        fclose(f);
        write(cfd, &fileSize, sizeof(fileSize));
        return;
    }

    read(cfd, &fileSize, sizeof(fileSize));

    while (received < fileSize && (received += read(cfd, buffer, sizeof(buffer))))
        fwrite(buffer, sizeof(unsigned char), sizeof(buffer), f);
        // print percantage and amount of bytes

    log_message(LOG_FILE, LOG_INFO, "Server's data successfully received");
    fclose(f);
}