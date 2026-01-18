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
        "  -h, --help             show this help and exit\n",
        prog);
}

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

int main(int argc, char *argv[]) {
    struct FilterOptions filters = {0};
    filters.size_max = LLONG_MAX; // size max muss mit einer hohen zahl
                        //initialisiert werden, sonst wird sie auf 0 gesetzt und dann failed das programm
    filters.max_depth = -1;

    int option_index = 0;
    int c;

    static const struct option long_options[] = {
        {"name", required_argument, 0, 'n'},
        {"suffix", required_argument, 0, 's'},
        {"type", required_argument, 0, 't'},
        {"size-min", required_argument, 0, 1000},
        {"size-max", required_argument, 0, 1001},
        {"maxdepth", required_argument, 0, 1002},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "n:s:t:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'n':
                filters.name = optarg;
                break;
            case 's':
                filters.suffix = optarg;
                break;
            case 't':
                filters.type = optarg[0];
                break;
            case 1000:
                if (parse_kb(optarg, &filters.size_min) != 0) {
                    fprintf(stderr, "Ungültiger Wert für size-min Option\n");
                    return 1;
                }
                break;
            case 1001:
                if (parse_kb(optarg, &filters.size_max) != 0) {
                    fprintf(stderr, "Ungültiger Wert für size-max Option\n");
                    return 1;
                }
                break;
            case 1002:
                if (parse_depth(optarg, &filters.max_depth) != 0) {
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

    if (filters.size_min > filters.size_max) {
        fprintf(stderr, "size-min must be <= size-max\n");
        return 1;
    }

    const char *path = NULL;
    if (optind < argc) {
        path = argv[optind];
    }

    if (path) {
        walk(path, &filters);
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
        walk(line, &filters);
    }
    if (ferror(stdin)) {
        perror("getline");
        free(line);
        return 1;
    }
    free(line);
    return 0;
}
