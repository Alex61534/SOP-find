#include <stdio.h>
#include <string.h>
#include "walk.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path> [-name filename]\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    const char *name_filter = NULL;

    if (argc >= 4 && strcmp(argv[2], "-name") == 0) {
        name_filter = argv[3];
    }

    walk(path, name_filter);
    return 0;
}
