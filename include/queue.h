#ifndef QUEUE_H
#define QUEUE_H

struct QueueItem {
    char *path;
    int depth;
    struct QueueItem *next;
};

typedef struct {
    struct QueueItem *head;
    struct QueueItem *tail;
} Queue;

void queue_init(Queue *q);
int queue_empty(const Queue *q);
void queue_push(Queue *q, const char *path, int depth);
struct QueueItem *queue_pop(Queue *q);

#endif
