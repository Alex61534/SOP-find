#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include "filters.h"
#include "walk.h"

typedef struct Node {
    char *path;
    int depth;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} Queue;

static void queue_init(Queue *q) {
    q->head=q->tail=NULL;
}

static int queue_empty(const Queue *q) {
    return q->head==NULL;
}

static void queue_push(Queue *q, const char *path, int depth) {
    Node *n = malloc(sizeof(Node));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    n->path = strdup(path);
    if (!n->path) {
        perror("strdup");
        free(n);
        exit(EXIT_FAILURE);
    }
    n->depth= depth;
    n->next=NULL;

    if(q->tail){
        q->tail->next=n;
    }else{
        q->head=n;
    }
    q->tail=n;	
}

static Node *queue_pop(Queue *q){
    if (queue_empty(q)){
        return NULL;
    }
    Node *n = q->head;
    q->head=n->next;
    if(!q->head){
        q->tail=NULL;
    }
    return n;
}

void walk (const char *path, const struct FilterOptions *filters){
    Queue q;
    queue_init(&q);
    queue_push(&q, path,0);

    while(!queue_empty(&q)){
        Node *node = queue_pop(&q);
        char *current = node->path;
        int depth = node->depth;

        struct stat st;
        if (lstat(current, &st) == -1){
            fprintf(stderr, "lstat failed on %s: %s\n",
                    current, strerror(errno));
            free(current);
            free(node);
            continue;
        }
        /* basename for name filter */
        const char *name = strrchr(current, '/');
        name = name ? name + 1 : current;

        if (filter_match_all(filters, name, &st)){
            printf("%s\n", current);
        }
        /* depth check before descending */
        if(S_ISDIR(st.st_mode)){
            if (filters->max_depth >= 0 && depth >= filters->max_depth) {
                free(current);
                free(node);
                continue;
            }
            DIR *dir = opendir(current);
            if(!dir){
                fprintf(stderr, "opendir failed on %s: %s\n",
                    current, strerror(errno));
                free(current);
                free(node);
                continue;
            }
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 ||
                    strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                char child[PATH_MAX];
                int needed = snprintf(child, sizeof(child),
                                      "%s/%s", current, entry->d_name);
                if (needed < 0 || needed >= (int)sizeof(child)) {
                    fprintf(stderr, "path too long: %s/%s\n",
                        current, entry->d_name);
                    continue;
                }

                queue_push(&q, child, depth + 1);
            }
            closedir(dir);
        }
        free(current);
        free(node);
    }
}
