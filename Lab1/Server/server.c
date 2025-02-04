#include "server.h"


SETTINGS* settings;
char* lastFile = NULL;

void run() {

    log_message(LOG_FILE, LOG_INFO, "Start server work");

    settings = init_settings();

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    char command[50];

    int sfd, cfd = -1;
    start_server(&sfd);
    
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    printf("> ");
    while (1) {
        
        if (cfd == -1) {
            cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientLen);
            if (cfd == -1 && !(errno == EWOULDBLOCK || errno == EAGAIN)) {
                log_message(LOG_FILE, LOG_CRITICAL, "Accept client error");
                close(sfd);
                close(cfd);
                exit(errno);
            } else if (cfd != -1)
                log_message(LOG_FILE, LOG_INFO, "Accept client connection"); // write also client addr??
        } else
            process_client(cfd);

        if (fgets(command, sizeof(command), stdin))  {
            command[strlen(command) - 1] = '\0';
            if (strcmp(command, "ECHO") == 0)
                echo();
            else if (strcmp(command, "TIME") == 0)
                server_time();
            else if(strstr(command, "SETTINGS") != 0) 
                settings_command(command); 
            else if (strcmp(command, "QUIT") == 0)
                break;
             printf("> ");
        }
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

    char buffer[80];
    if (read(cfd, buffer, sizeof(buffer)) == -1)
        return;
    if (strstr(buffer, "UPLOAD") != NULL) {
        char* filePath = buffer + 7;
        receive_data(cfd, filePath);
    } else if (strstr(buffer, "DOWNLOAD") != NULL) {
        char* filePath = buffer + 9;
        send_data(cfd, filePath);
    }
}

void receive_data(int cfd, const char* filePath) {

    log_message(LOG_FILE, LOG_INFO, "Start receive client data");
    // process lost connection
    unsigned char buffer[80];
    int fileSize = 0, received = 0;

    char* fileName = get_filename(filePath);
    char* serverFilePath = get_file_path(settings->file_path, fileName);
    FILE* f = fopen(serverFilePath, "wb");
    if (f == NULL) {
        log_message(LOG_FILE, LOG_ERROR, "Can't create file to receive data from client");
        fclose(f);
        write(cfd, &fileSize, sizeof(fileSize));
        return;
    }

    read(cfd, &fileSize, sizeof(fileSize));

    int rec;
    while (received < fileSize && (rec = read(cfd, buffer, sizeof(buffer)))) { 
        fwrite(buffer, sizeof(unsigned char), rec, f);
        received += rec;
    }
        // print percantage and amount of bytes

    log_message(LOG_FILE, LOG_INFO, "Client's data successfully received");
    fclose(f);
    free(serverFilePath);
    free(fileName);
}

void send_data(int cfd, char* filePath) {

    log_message(LOG_FILE, LOG_INFO, "Start send data to client");
    // process lost connection
    unsigned char buffer[80];
    int fileSize = -1, sent = 0, bytesRead;

    char* serverFilePath = filePath;
    if(is_absolute_path(filePath) != 1) {
        serverFilePath = get_file_path(settings->file_path, filePath);
    }

    FILE* f = fopen(serverFilePath, "rb");
    if (f == NULL) {
        log_message(LOG_FILE, LOG_ERROR, "Can't open file to send data to client");
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

    int read;
    while (sent < fileSize && (read = fread(buffer, 1, sizeof(buffer), f))) {
        write(cfd, buffer, read);
        sent += read;
    }

    printf("File successfully sent to client.\n>");

    log_message(LOG_FILE, LOG_INFO, "Data successfully sent to client");
    fclose(f);
}

void echo() {
    if (lastFile != NULL)
        printf("Last processed file: %s\n", lastFile);
    else
        printf("No file processed before\n");
    log_message(LOG_FILE, LOG_INFO, "Process command ECHO");
}

void server_time() {
    time_t now = time(NULL);
    struct tm* timeInfo = localtime(&now);

    printf("%02d.%02d.%4d %2d:%2d:%2d\n", 
        timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900,
        timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    log_message(LOG_FILE, LOG_INFO, "Process command TIME");
}

//__________________ SETTINGS _________________________

void settings_command(char* command) {
    if(strstr(command, ".path") != 0) {
        char* start_i = strchr(command, ' ');
        if(start_i == NULL) 
            return;

        int last_i = strlen(command) - 1;

        while(*(++start_i) == ' ');
 
        char dir[MAX_PATH_SIZE];
        if(*start_i == '"' && command[last_i] == '"') {
            start_i++; last_i--; 
            strncpy(dir, start_i, strlen(start_i));
            dir[strlen(start_i) - 1] = '\0';
        } else { 
            strcpy(dir, start_i);
        }
        
        settings_cmd(settings, SET_PATH, dir);

    } else if(strcmp(command, "SETTINGS") == 0) { 
        settings_cmd(settings, SETTINGS_LIST, NULL);
    }
}