#include <stdio.h>
#include <stdlib.h>
#include "walk.h" 

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <start-directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    walk(argv[1]);
    return EXIT_SUCCESS;
}

