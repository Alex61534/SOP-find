#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "walk.h"


void walk(const char *path, const char *name_filter) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {

        // "." und ".." überspringen
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // neuen Pfad bauen
        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        // Dateityp bestimmen
        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror(fullpath);
            continue;
        }

        // Pfad ausgeben, optional mit Name-Filter
        if (name_filter == NULL || strcmp(entry->d_name, name_filter) == 0) {
            printf("%s\n", fullpath);
        }

        // Wenn Verzeichnis → rekursiv weiterlaufen
        if (S_ISDIR(st.st_mode)) {
            walk(fullpath, name_filter);
        }
    }

    closedir(dir);
}
