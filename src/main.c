#include <stdio.h>
#include <string.h>
#include "filters.h"
#include "walk.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path> [-name filename]\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    struct FilterOptions filters = {0};

    if (argc >= 4 && strcmp(argv[2], "-name") == 0) {
        filters.name = argv[3];
    }

    walk(path, &filters);
    return 0;
}
