#ifndef IO_H
#define IO_H

#include "filters.h"

char *normalize_path(const char *path);
int walk_from_stdin(const struct Options *opts);

#endif
