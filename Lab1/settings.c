#include "settings.h"

SETTINGS* init_settings() {
    SETTINGS* settings = (SETTINGS*)malloc(sizeof(SETTINGS));
    strcpy(settings->file_path, ".");
    return settings;
}

int settings_cmd(SETTINGS* settings, const char* command, const char* value) {
    int res = 1;
    if(strcmp(command, SET_PATH) == 0) {
        res = set_path(settings, value);
    } else if(strcmp(command, SETTINGS_LIST) == 0) {
        printf("PATH: %s\n", settings->file_path);
    } 

    return res;
}

int set_path(SETTINGS* settings, const char* dir) {
    int fd;
    if(!is_directory_exists(dir)) {
        fd = mkdir(dir, 0755);
        printf("%d %s", fd, dir);
        if(fd == -1)
            return 0;
    }
    strcpy(settings->file_path, dir);
    return 1;
}

int is_directory_exists(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0) {
        return 0; 
    }
    return (info.st_mode & __S_IFDIR) != 0; 
}

char *get_file_path(const char *folder, const char *filename) {
    if (folder == NULL || filename == NULL) {
        return NULL;
    }

    size_t folder_len = strlen(folder);
    size_t filename_len = strlen(filename);
    size_t separator_len = (folder_len > 0 && folder[folder_len - 1] != '/') ? 1 : 0;

    char *full_path = malloc(folder_len + filename_len + separator_len + 1);
    
    if (full_path == NULL) {
        return NULL; 
    }

    if (separator_len) {
        sprintf(full_path, "%s/%s", folder, filename);
    } else {
        sprintf(full_path, "%s%s", folder, filename);
    }

    return full_path;
}

int is_absolute_path(const char* path) {
    if(path == NULL && strlen(path) == 0) {
        return -1;
    }

    return path[0] == '/'? 1 : 0;
}

char* get_filename(const char *path) {
    if (path == NULL) {
        return NULL;
    }
    
    const char *filename = strrchr(path, '/');
    
    if (filename != NULL) {
        return strdup(filename + 1); 
    } else {
        return strdup(path); 
    }
}