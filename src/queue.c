#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

void queue_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
}

int queue_empty(const Queue *q) {
    return q->head == NULL;
}

void queue_push(Queue *q, const char *path, int depth) {
    struct QueueItem *n = malloc(sizeof(struct QueueItem));
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
    n->depth = depth;
    n->next = NULL;

    if (q->tail) {
        q->tail->next = n;
    } else {
        q->head = n;
    }
    q->tail = n;
}

struct QueueItem *queue_pop(Queue *q) {
    if (queue_empty(q)) {
        return NULL;
    }
    struct QueueItem *n = q->head;
    q->head = n->next;
    if (!q->head) {
        q->tail = NULL;
    }
    return n;
}
