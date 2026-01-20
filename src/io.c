#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "walk.h"

char *normalize_path(const char *path) {
    size_t len = strlen(path);

    if (len <= 1 || path[len - 1] != '/') {
        return strdup(path);
    }

    char *new_path = malloc(len); // len statt len+1, weil ein Zeichen wegfällt
    if (!new_path) {
        return NULL;
    }

    memcpy(new_path, path, len - 1);
    new_path[len - 1] = '\0';

    return new_path;
}

int walk_from_stdin(const struct Options *opts) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    while ((len = getline(&line, &cap, stdin)) != -1) {
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) {
            continue;
        }
        char *normalized = normalize_path(line);
        if (!normalized) {
            free(line);
            return 1;
        }
        walk(normalized, opts);
        free(normalized);
    }
    if (ferror(stdin)) {
        perror("getline");
        free(line);
        return 1;
    }
    free(line);
    return 0;
}
