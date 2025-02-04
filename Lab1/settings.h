#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_SIZE       4096
#define MAX_FILE_NAME_SIZE  255

#define SET_PATH        "PATH"
#define SETTINGS_LIST   "LIST"

typedef struct _SETTINGS {
    char file_path[MAX_PATH_SIZE];
} SETTINGS;

SETTINGS* init_settings();
int settings_cmd(SETTINGS* settings, const char* command, const char* value);
int set_path(SETTINGS* settings, const char* dir);
int is_directory_exists(const char *path);
char *get_file_path(const char *folder, const char *filename);
int is_absolute_path(const char* path);
char* get_filename(const char *path);

#endif // SETTINGS_H