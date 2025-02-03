#include "logger.h"

void log_message(const char* fileName, const char* level, const char* message) {

    FILE* f = fopen(fileName, "a");
    if (f == NULL) {
        fclose(f);
        perror("Error opening log file.\n");
        exit(errno);
    }

    time_t now = time(NULL);
    struct tm* timeInfo = localtime(&now);

    fprintf(f, "[%2d %2d %4d %2d:%2d:%2d] [%8s] [%s]\n", 
        timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900,
        timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec,
        level, message);

    fclose(f);
}