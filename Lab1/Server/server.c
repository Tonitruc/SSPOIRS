#include "server.h"

char* lastFile = NULL;

void run() {

    log_message(LOG_FILE, LOG_INFO, "Start server work");

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    char command[10];

    int sfd, cfd;
    start_server(&sfd);
    
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    while (1) {
        
        printf("fuck0");
        cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientLen);
        if (cfd == -1 && !(errno == EWOULDBLOCK || errno == EAGAIN)) {
            log_message(LOG_FILE, LOG_CRITICAL, "Accept client error");
            close(sfd);
            close(cfd);
            exit(errno);
        } else if (cfd != -1) {
            log_message(LOG_FILE, LOG_INFO, "Accept client connection"); // write also client addr??
            process_client(cfd);
        }
        printf("fuck1");

        if (fgets(command, sizeof(command), stdin))  {
            if (strcmp(command, "ECHO") == 0)
                echo();
            else if (strcmp(command, "TIME") == 0)
                server_time();
            else if (strcmp(command, "QUIT") == 0)
                break;
        }
        printf("fuck2");
    }

    log_message(LOG_FILE, LOG_INFO, "Stop server work");
    fcntl(STDIN_FILENO, F_SETFL, flags);
    close(sfd);
}

void start_server(int* sfd) {

    *sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sfd == -1) {
        log_message(LOG_FILE, LOG_CRITICAL, "Can't open socket");
        close(*sfd);
        exit(errno);
    }
    log_message(LOG_FILE, LOG_INFO, "Open socket");

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(*sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    int opt = 1;
    if(setsockopt(*sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        log_message(LOG_FILE, LOG_CRITICAL, "Can't set socket options");
        close (*sfd);
        exit(errno);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(*sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        log_message(LOG_FILE, LOG_CRITICAL, "Can't bind socket");
        close(*sfd);
        exit(errno);
    }
    log_message(LOG_FILE, LOG_INFO, "Bing server");

    if (listen(*sfd, 5) == -1) {
        log_message(LOG_FILE, LOG_CRITICAL, "Can't start listen");
        close(*sfd);
        exit(errno);
    }
    log_message(LOG_FILE, LOG_INFO, "Server start to listen");

    fcntl(*sfd, F_SETFL, O_NONBLOCK);
}

void process_client(int cfd) {

    while (1) {

        char buffer[80];
        read(cfd, buffer, sizeof(buffer));
        // continue operation if disconnect
        if (strstr(buffer, "UPLOAD") != NULL) {
            char* file = buffer + 7;
            receive_data(cfd, file);
        } else if (strstr(buffer, "DOWNLOAD") != NULL) {
            char* file = buffer + 9;
            send_data(cfd, file);
        }
    }
}

void receive_data(int cfd, const char* file) {

    log_message(LOG_FILE, LOG_INFO, "Start receive client data");
    // process lost connection
    unsigned char buffer[80];
    int fileSize = 0, received = 0;

    FILE* f = fopen(file, "wb");
    if (f == NULL) {
        log_message(LOG_FILE, LOG_ERROR, "Can't create file to receive data from client");
        fclose(f);
        write(cfd, &fileSize, sizeof(fileSize));
        return;
    }

    read(cfd, &fileSize, sizeof(fileSize));

    while (received < fileSize && (received += read(cfd, buffer, sizeof(buffer))))
        fwrite(buffer, sizeof(unsigned char), sizeof(buffer), f);
        // print percantage and amount of bytes

    log_message(LOG_FILE, LOG_INFO, "Client's data successfully received");
    fclose(f);
}

void send_data(int cfd, const char* file) {

    log_message(LOG_FILE, LOG_INFO, "Start send data to client");
    // process lost connection
    unsigned char buffer[80];
    int fileSize = -1, sent = 0, bytesRead;

    FILE* f = fopen(file, "rb");
    if (f == NULL) {
        log_message(LOG_FILE, LOG_ERROR, "Can't open file to send data to client");
        fclose(f);
        write(cfd, &fileSize, sizeof(fileSize));
        return;
    }

    if (read(cfd, &fileSize, sizeof(fileSize)) >= 0 && fileSize == -1) {
        log_message(LOG_FILE, LOG_ERROR, "Can't open file on client");
        fclose(f);
        return;
    }

    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    rewind(f);
    write (cfd, &fileSize, sizeof(fileSize));

    while (sent < fileSize && (sent += fread(buffer, 1, sizeof(buffer), f))) {
        write(cfd, buffer, sizeof(buffer));
        // display persantage and amount of sent bytes
    }

    log_message(LOG_FILE, LOG_INFO, "Data successfully sent to client");
    fclose(f);
}

void echo() {
    log_message(LOG_FILE, LOG_INFO, "Process command ECHO");
    if (lastFile != NULL)
        printf("Last processed file: %s\n", lastFile);
    else
        printf("No file processed before\n");
}

void server_time() {
    log_message(LOG_FILE, LOG_INFO, "Process command TIME");
    time_t now = time(NULL);
    struct tm* timeInfo = localtime(&now);

    printf("%2d %2d %4d %2d:%2d:%2d\n", 
        timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900,
        timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
}