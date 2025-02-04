#include "server.h"

char* lastFile = NULL;

void run() {

    log_message(LOG_FILE, LOG_INFO, "Start server work");

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    char command[10];

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
            else if (strcmp(command, "QUIT") == 0)
                break;
            else if(strcmp(command, "MHIF") == 0) {
                mhif_command();
            }
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
    timeout.tv_sec = 3;
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
        char* file = buffer + 7;
        receive_data(cfd, file);
    } else if (strstr(buffer, "DOWNLOAD") != NULL) {
        char* file = buffer + 9;
        send_data(cfd, file);
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

    int rec;
    while (received < fileSize && (rec = read(cfd, buffer, sizeof(buffer)))) {
        fwrite(buffer, sizeof(unsigned char), rec, f);
        received += rec;
    }
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

    int read;
    while (sent < fileSize && (read = fread(buffer, 1, sizeof(buffer), f))) {
        write(cfd, buffer, read);
        sent += read;
        // display persantage and amount of sent bytes
    }

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

//___________________ MINF ________________________________
// need packages << libcurl4-openssl-dev libmpg123-dev >>



size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void mhif_command() {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    const char *url = "https://drive.google.com/uc?export=download&id=17Toso8JcPQpizFuicb9AbBSGzvD2mqev"; 
    const char *outfilename = "dechland.mp3";

    // Инициализация curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        fp = fopen(outfilename, "wb");
        if (!fp) {
            fprintf(stderr, "Не удалось открыть файл для записи\n");
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        fclose(fp);
        
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return;
        }
        
        curl_easy_cleanup(curl);
    }

    play_mp3(outfilename);
}

void play_mp3(const char *filename) {
    mpg123_handle *mh;
    unsigned char buffer[BUFFER_SIZE];
    size_t done;
    int err;

    // Инициализация mpg123
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    if (mh == NULL) {
        fprintf(stderr, "Не удалось инициализировать mpg123: %s\n", mpg123_strerror(mh));
        return;
    }

    // Открытие MP3 файла
    if (mpg123_open(mh, filename) != MPG123_OK) {
        fprintf(stderr, "Не удалось открыть MP3 файл: %s\n", mpg123_strerror(mh));
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Получение информации о частоте и каналах
    long rate;
    int channels, encoding;
    mpg123_getformat(mh, &rate, &channels, &encoding);

    // Инициализация ALSA
    snd_pcm_t *pcm_handle;
    snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, channels, rate, 1, 500000);

    // Воспроизведение MP3
    while ((err = mpg123_read(mh, buffer, BUFFER_SIZE, &done)) == MPG123_OK) {
        snd_pcm_sframes_t frames = snd_pcm_writei(pcm_handle, buffer, done / (2 * channels)); // 2 байта на канал
        if (frames < 0) {
            frames = snd_pcm_recover(pcm_handle, frames, 0);
        }
    }

    // Освобождение ресурсов
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}
