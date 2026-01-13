#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "filters.h"
#include "walk.h"

void walk(const char *path, const struct FilterOptions *filters) {
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

        // Pfad ausgeben, optional mit Name-Filter oder suffix
        if (filter_match_name(entry->d_name, filters)) {
            printf("%s\n", fullpath);
        }

        // Wenn Verzeichnis → rekursiv weiterlaufen
        if (S_ISDIR(st.st_mode)) {
            walk(fullpath, filters);
        }
    }

    closedir(dir);
}
