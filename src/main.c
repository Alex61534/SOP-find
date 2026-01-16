#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "walk.h"
#include "filters.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path> [-name filename]\n", argv[0]);
        return 1;
    }

    struct FilterOptions filters = {0};
    filters.size_max = LLONG_MAX; // size max muss mit einer hohen zahl
                        //initialisiert werden, sonst wird sie auf 0 gesetzt und dann failed das programm

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
        } //size wird immer in kb angegeben
        else if (strcmp(argv[i], "-size-min") == 0) {
            if (i+ 1 >= argc) {
                fprintf(stderr, "Sie brauchen ein Argument für die size-min Option");
                return 1;
            }

            long kb = strtol(argv[i +1], NULL, 10);

            if(kb < 0){
                fprintf(stderr, "Ungültiger Wert für size Option");
                return 1;
            }
            filters.size_min = kb * 1024;
        }
        else if (strcmp(argv[i], "-size-max") == 0) {
            if (i+ 1 >= argc) {
                fprintf(stderr, "Sie brauchen ein Argument für die size-max Option");
                return 1;
            }

            long kb = strtol(argv[i +1], NULL, 10);

            if(kb < 0){
                fprintf(stderr, "Ungültiger Wert für size Option");
                return 1;
            }
            filters.size_max = kb * 1024;
        }
        else if (strcmp(argv[i], "-type") == 0) {
            if (i+ 1 >= argc) {
                fprintf(stderr, "Sie brauchen ein Argument für die type Option");
                return 1;
            }
            filters.type = argv[i + 1][0];
        }
        else{
                            fprintf(stderr, "Unbekannte Filteroption");
                            return 1;
        }

    }

    walk(path, &filters);
    return 0;
}