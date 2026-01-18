#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "filters.h"
#include "walk.h"

static void walk_internal(const char *path, const struct FilterOptions *filters, int depth) {
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

        // Pfad ausgeben, optional mit Name-Filter oder suffix, min mx size
        if (filter_match_name(entry->d_name, filters, &st)) {
            printf("%s\n", fullpath);
        }

        // Wenn Verzeichnis → rekursiv weiterlaufen
        if (S_ISDIR(st.st_mode)) {
            if (filters == NULL || filters->max_depth < 0 || depth < filters->max_depth) {
                walk_internal(fullpath, filters, depth + 1);
            }
        }
    }

    closedir(dir);
}

void walk(const char *path, const struct FilterOptions *filters) {
    walk_internal(path, filters, 0);
}
