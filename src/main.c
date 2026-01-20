#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cli.h"
#include "io.h"
#include "walk.h"

int main(int argc, char *argv[]) {
    struct Options opts = {0};
    // Default: no max size limit unless specified.
    opts.filters.size_max = LLONG_MAX; // size max muss mit einer hohen zahl
                        //initialisiert werden, sonst wird sie auf 0 gesetzt und dann failed das programm
    // Default: no depth limit.
    opts.filters.max_depth = -1;

    const char *path = NULL;
    int parse_status = parse_cli(argc, argv, &opts, &path);
    if (parse_status == CLI_HELP) {
        return 0;
    }
    if (parse_status != CLI_OK) {
        return 1;
    }

    if (opts.actions.delete_mode && !isatty(STDIN_FILENO)) {
        fprintf(stderr, "--delete requires an interactive terminal for confirmation\n");
        return 1;
    }

    if (path) {
        char *normalized = normalize_path(path);
        if (!normalized) {
            return 1;
        }
        walk(normalized, &opts);
        free(normalized);
        return 0;
    }

    if (isatty(STDIN_FILENO)) {
        cli_print_usage(argv[0]);
        return 1;
    }

    return walk_from_stdin(&opts);
}
