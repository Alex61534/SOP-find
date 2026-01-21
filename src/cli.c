#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cli.h"

void cli_print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <path> [options]\n"
        "Options:\n"
        "  -n, --name NAME          exact filename match\n"
        "  -s, --suffix SUFFIX      file suffix (case-insensitive, without dot)\n"
        "  -t, --type TYPE          file type (e.g. f, d, l)\n"
        "  -c, --name-contains NAME filename contains name\n"
        "  -e, --exclude NAME       filename can not contain name\n"
        "      --size-min KB        minimum size in KB\n"
        "      --size-max KB        maximum size in KB\n"
        "      --maxdepth N         maximum recursion depth (0 = only start dir)\n"
        "  -D, --delete             prompt before deleting matches (files only)\n"
        "  -h, --help               show this help and exit\n",
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

int parse_cli(int argc, char **argv, struct Options *opts, const char **path) {
    int option_index = 0;
    int c;

    static const struct option long_options[] = {
        {"name", required_argument, 0, 'n'},
        {"name-contains", required_argument, 0, 'c'},
        {"exclude", required_argument, 0, 'e'},
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
                opts->filters.name = optarg;
                break;
            case 's':
                opts->filters.suffix = optarg;
                break;
            case 't':
                opts->filters.type = optarg[0];
                break;
            case 'c':
                opts->filters.name_contains = optarg;
                break;
            case 'e':
                opts->filters.exclude = optarg;
                break;
            case 'D':
                opts->actions.delete_mode = true;
                break;
            case 1000:
                if (parse_kb(optarg, &opts->filters.size_min) != 0) {
                    fprintf(stderr, "Ungültiger Wert für size-min Option\n");
                    return CLI_ERROR;
                }
                break;
            case 1001:
                if (parse_kb(optarg, &opts->filters.size_max) != 0) {
                    fprintf(stderr, "Ungültiger Wert für size-max Option\n");
                    return CLI_ERROR;
                }
                break;
            case 1002:
                if (parse_depth(optarg, &opts->filters.max_depth) != 0) {
                    fprintf(stderr, "Ungültiger Wert für maxdepth Option\n");
                    return CLI_ERROR;
                }
                break;
            case 'h':
                cli_print_usage(argv[0]);
                return CLI_HELP;
            default:
                cli_print_usage(argv[0]);
                return CLI_ERROR;
        }
    }

    if (opts->filters.size_min > opts->filters.size_max) {
        fprintf(stderr, "size-min must be <= size-max\n");
        return CLI_ERROR;
    }

    if (optind < argc) {
        *path = argv[optind];
    }
    return CLI_OK;
}
