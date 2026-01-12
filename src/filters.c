#include <string.h>
#include "filters.h"

bool filter_match_name(const char *entry_name, const struct FilterOptions *filters) {
    if (filters == NULL || filters->name == NULL) {
        return true;
    }
    return strcmp(entry_name, filters->name) == 0;
}
