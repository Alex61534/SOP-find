#ifndef FILTERS_H
#define FILTERS_H

#include <stdbool.h>

struct FilterOptions {
    const char *name;
};

bool filter_match_name(const char *entry_name, const struct FilterOptions *filters);

#endif
