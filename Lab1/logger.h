#include <stdio.h>
#include <time.h>
#include <errno.h>

#define LOG_INFO     "INFO"
#define LOG_ERROR    "ERROR"
#define LOG_CRITICAL "CRITICAL"

void log_message(const char* fileName, const char* level, const char* message);