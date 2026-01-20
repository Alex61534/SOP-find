#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include "walk.h"
#include "filters.h"

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <path> [options]\n"
        "Options:\n"
        "  -n, --name NAME        exact filename match\n"
        "  -s, --suffix SUFFIX    file suffix (case-insensitive, without dot)\n"
        "  -t, --type TYPE        file type (e.g. f, d, l)\n"
        "      --size-min KB      minimum size in KB\n"
        "      --size-max KB      maximum size in KB\n"
        "      --maxdepth N       maximum recursion depth (0 = only start dir)\n"
        "  -D, --delete           prompt before deleting matches (files only)\n"
        "  -h, --help             show this help and exit\n",
        prog);
}

// Parse size arguments in KB, reject negatives and overflow.
static int parse_kb(const char *arg, off_t *out) {
    char *end = NULL;
    errno = 0;
    long long kb = strtoll(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' || kb < 0) {
        return -1;
    }
    unsigned int bits = sizeof(off_t) * CHAR_BIT;
    unsigned long long max_off;
    if (bits >= 63) {
        max_off = (unsigned long long)LLONG_MAX;
    } else {
        max_off = (1ULL << (bits - 1)) - 1ULL;
    }
    if ((unsigned long long)kb > max_off / 1024ULL) {
        return -1;
    }
    *out = (off_t)kb * 1024;
    return 0;
}

// Parse recursion depth; 0 means only the start directory.
static int parse_depth(const char *arg, int *out) {
    char *end = NULL;
    errno = 0;
    long depth = strtol(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' || depth < 0) {
        return -1;
    }
    if (depth > INT_MAX) {
        return -1;
    }
    *out = (int)depth;
    return 0;
}

char *checkEnding(const char *path) {
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

int main(int argc, char *argv[]) {
    struct Options opts = {0};
    // Default: no max size limit unless specified.
    opts.filters.size_max = LLONG_MAX; // size max muss mit einer hohen zahl
                        //initialisiert werden, sonst wird sie auf 0 gesetzt und dann failed das programm
    // Default: no depth limit.
    opts.filters.max_depth = -1;

    int option_index = 0;
    int c;

    // Long options for more readable CLI usage.
    static const struct option long_options[] = {
        {"name", required_argument, 0, 'n'},
        {"suffix", required_argument, 0, 's'},
        {"type", required_argument, 0, 't'},
        {"size-min", required_argument, 0, 1000},
        {"size-max", required_argument, 0, 1001},
        {"maxdepth", required_argument, 0, 1002},
        {"delete", no_argument, 0, 'D'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "n:s:t:Dh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'n':
                opts.filters.name = optarg;
                break;
            case 's':
                opts.filters.suffix = optarg;
                break;
            case 't':
                opts.filters.type = optarg[0];
                break;
            case 'D':
                opts.actions.delete_mode = true;
                break;
            case 1000:
                if (parse_kb(optarg, &opts.filters.size_min) != 0) {
                    fprintf(stderr, "Ungültiger Wert für size-min Option\n");
                    return 1;
                }
                break;
            case 1001:
                if (parse_kb(optarg, &opts.filters.size_max) != 0) {
                    fprintf(stderr, "Ungültiger Wert für size-max Option\n");
                    return 1;
                }
                break;
            case 1002:
                if (parse_depth(optarg, &opts.filters.max_depth) != 0) {
                    fprintf(stderr, "Ungültiger Wert für maxdepth Option\n");
                    return 1;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (opts.filters.size_min > opts.filters.size_max) {
        fprintf(stderr, "size-min must be <= size-max\n");
        return 1;
    }

    const char *path = NULL;
    if (optind < argc) {
        path = argv[optind];
    }

    if (opts.actions.delete_mode && !isatty(STDIN_FILENO)) {
        fprintf(stderr, "--delete requires an interactive terminal for confirmation\n");
        return 1;
    }

    if (path) {
        char *normalized = checkEnding(path);
        if (!normalized) {
            return 1;
        }
        walk(normalized, &opts);
        free(normalized);
        return 0;
    }

    if (isatty(STDIN_FILENO)) {
        print_usage(argv[0]);
        return 1;
    }

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
        char *normalized = checkEnding(line);
        if (!normalized) {
            free(line);
            return 1;
        }
        walk(normalized, &opts);
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
