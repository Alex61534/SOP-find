#include <stdio.h>
#include <string.h>
#include "walk.h"
#include "filters.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path> [-name filename]\n", argv[0]);
        return 1;
    }

    struct FilterOptions filters = {0};

    const char *path = argv[1];

    for(int i = 2; i < argc; i += 2) {
        if (strcmp(argv[i], "-name") == 0){
            if (i + 1 >= argc) {
                fprintf(stderr, "Sie brauchen ein Argument für die name Option");
                return 1;
            }
            filters.name = argv[i + 1];
        } 
        else if (strcmp(argv[i], "-suffix") == 0) {
            if (i+ 1 >= argc) {
                fprintf(stderr, "Sie brauchen ein Argument für die suffix Option");
                return 1;
            }
            filters.suffix = argv[i + 1];
        }
        else{
                            fprintf(stderr, "Unbekannte Filteroption");
                            return 1;
        }

    }

    walk(path, &filters);
    return 0;
}