#ifndef FILTERS_H
#define FILTERS_H

#include <stdbool.h>
#include <sys/types.h>  
#include <sys/stat.h> // für off_t

struct FilterOptions {
    const char *name;
    const char *suffix;
    char type;

    off_t size_min;             //off_t unterstützt große dateien und ist stat() kompatibel
    off_t size_max;
};

bool filter_match_name(const char *entry_name,
                    const struct FilterOptions *filters,
                    const struct stat *st);
bool match_suffix(const char *entry_name, const char *suffix);

#endif
