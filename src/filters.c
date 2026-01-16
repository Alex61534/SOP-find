#include <string.h>
#include <strings.h>
#include "filters.h"
#include <sys/types.h>
#include <sys/stat.h>

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

bool match_type(const struct stat *st, const char type){
    if(type == '\0'){ //da type einzelnes char darf man nicht mit null vergleichen
        return true;
    }

    switch(type){
    case 'f': return S_ISREG(st->st_mode);//normale datei
    case 'd': return S_ISDIR(st->st_mode);//verzeichnis
    case 'l': return S_ISLNK(st->st_mode);//symbolischer Link
    case 'c': return S_ISCHR(st->st_mode);//character device
    case 'b': return S_ISBLK(st->st_mode);//block device
    case 'p': return S_ISFIFO(st->st_mode);//named pipe
    case 's': return S_ISSOCK(st->st_mode);//unix domain socket
    default:  return false;
    }
}

bool filter_match_name(const char *entry_name,
                    const struct FilterOptions *filters,
                    const struct stat *st) {
    if (filters == NULL){
        return true;
    }

    if(filters->name != NULL && strcmp(entry_name, filters->name) != 0){
        return false;
    }

    if (!match_suffix(entry_name, filters->suffix)){
        return false;
    }

    if(st->st_size < filters->size_min){
        return false;
    }

    if(st->st_size > filters->size_max){
        return false;
    }
    if(!match_type(st, filters->type)){
        return false;
    }

    return true;
}