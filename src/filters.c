#include <string.h>
#include <strings.h>
#include "filters.h"

bool match_suffix(const char *entry_name, const char *suffix){
    if(suffix == NULL){
        return true;
    }

    const char *dot = strrchr(entry_name, '.');
    if(dot == NULL){
        return false;
    }

    return strcasecmp(dot + 1, suffix) == 0;
}

bool filter_match_name(const char *entry_name, const struct FilterOptions *filters) {
    if (filters == NULL){
        return true;
    }

    if(filters->name != NULL && strcmp(entry_name, filters->name) != 0){
        return false;
    }

    if (!match_suffix(entry_name, filters->suffix)){
        return false;
    }

    return true;
}