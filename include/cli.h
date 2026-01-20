#ifndef CLI_H
#define CLI_H

#include "filters.h"

#define CLI_OK 0
#define CLI_ERROR 1
#define CLI_HELP 2

void cli_print_usage(const char *prog);
int parse_cli(int argc, char **argv, struct Options *opts, const char **path);

#endif
