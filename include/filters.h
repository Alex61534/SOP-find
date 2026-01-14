#ifndef FILTERS_H
#define FILTERS_H

#include <stdbool.h>

struct FilterOptions {
    const char *name;
    const char *suffix;
    int max_depth;
};

bool filter_match_name(const char *entry_name, const struct FilterOptions *filters);
bool match_suffix(const char *entry_name, const char *suffix);

#endif
