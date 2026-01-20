#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include "filters.h"
#include "queue.h"
#include "walk.h"

typedef struct {
    Queue q;
    pthread_mutex_t mu;
    pthread_cond_t cv;
    size_t pending;
    int done;
} WorkQueue;

typedef struct {
    WorkQueue *wq;
    const struct Options *options;
    pthread_mutex_t *io_mu;
} WorkerCtx;

static void log_msg(pthread_mutex_t *io_mu, FILE *stream, const char *fmt, ...) {
    va_list ap;

    pthread_mutex_lock(io_mu);
    va_start(ap, fmt);
    vfprintf(stream, fmt, ap);
    va_end(ap);
    pthread_mutex_unlock(io_mu);
}

static int confirm_delete(const char *path, pthread_mutex_t *io_mu) {
    char buf[16];
    int confirmed = 0;

    pthread_mutex_lock(io_mu);
    fprintf(stderr, "Delete %s? [y/N]: ", path);
    fflush(stderr);
    if (fgets(buf, sizeof(buf), stdin)) {
        for (size_t i = 0; buf[i] != '\0'; i++) {
            if (isspace((unsigned char)buf[i])) {
                continue;
            }
            confirmed = (buf[i] == 'y' || buf[i] == 'Y');
            break;
        }
    }
    pthread_mutex_unlock(io_mu);
    return confirmed;
}

static void workqueue_init(WorkQueue *wq) {
    queue_init(&wq->q);
    pthread_mutex_init(&wq->mu, NULL);
    pthread_cond_init(&wq->cv, NULL);
    wq->pending = 0;
    wq->done = 0;
}

static void workqueue_destroy(WorkQueue *wq) {
    pthread_mutex_destroy(&wq->mu);
    pthread_cond_destroy(&wq->cv);
}

static void workqueue_push(WorkQueue *wq, const char *path, int depth) {
    pthread_mutex_lock(&wq->mu);
    queue_push(&wq->q, path, depth);
    wq->pending++;
    pthread_cond_signal(&wq->cv);
    pthread_mutex_unlock(&wq->mu);
}

static struct QueueItem *workqueue_pop(WorkQueue *wq) {
    struct QueueItem *node = NULL;

    pthread_mutex_lock(&wq->mu);
    while (queue_empty(&wq->q) && !wq->done) {
        pthread_cond_wait(&wq->cv, &wq->mu);
    }
    if (!wq->done) {
        node = queue_pop(&wq->q);
    }
    pthread_mutex_unlock(&wq->mu);
    return node;
}

static void workqueue_task_done(WorkQueue *wq) {
    pthread_mutex_lock(&wq->mu);
    if (wq->pending > 0) {
        wq->pending--;
    }
    if (wq->pending == 0) {
        wq->done = 1;
        pthread_cond_broadcast(&wq->cv);
    }
    pthread_mutex_unlock(&wq->mu);
}

static void *worker_main(void *arg) {
    WorkerCtx *ctx = arg;
    const struct Options *options = ctx->options;
    const struct FilterOptions *filters = options ? &options->filters : NULL;

    for (;;) {
        struct QueueItem *node = workqueue_pop(ctx->wq);
        if (!node) {
            break;
        }
        char *current = node->path;
        int depth = node->depth;

        struct stat st;
        if (lstat(current, &st) == -1) {
            log_msg(ctx->io_mu, stderr, "lstat failed on %s: %s\n",
                    current, strerror(errno));
            free(current);
            free(node);
            workqueue_task_done(ctx->wq);
            continue;
        }
        /* basename for name filter */
        const char *name = strrchr(current, '/');
        name = name ? name + 1 : current;

        if (filter_match_all(filters, name, &st)) {
            if (options && options->actions.delete_mode) {
                if (S_ISREG(st.st_mode) && confirm_delete(current, ctx->io_mu)) {
                    if (unlink(current) == 0) {
                        log_msg(ctx->io_mu, stdout, "deleted %s\n", current);
                    } else {
                        log_msg(ctx->io_mu, stderr, "delete failed on %s: %s\n",
                                current, strerror(errno));
                    }
                }
            } else {
                log_msg(ctx->io_mu, stdout, "%s\n", current);
            }
        }

        /* depth check before descending */
        if (S_ISDIR(st.st_mode)) {
            if (filters && filters->max_depth >= 0 && depth >= filters->max_depth) {
                free(current);
                free(node);
                workqueue_task_done(ctx->wq);
                continue;
            }
            DIR *dir = opendir(current);
            if (!dir) {
                log_msg(ctx->io_mu, stderr, "opendir failed on %s: %s\n",
                        current, strerror(errno));
                free(current);
                free(node);
                workqueue_task_done(ctx->wq);
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
                    log_msg(ctx->io_mu, stderr, "path too long: %s/%s\n",
                            current, entry->d_name);
                    continue;
                }

                workqueue_push(ctx->wq, child, depth + 1);
            }
            closedir(dir);
        }

        free(current);
        free(node);
        workqueue_task_done(ctx->wq);
    }
    return NULL;
}

void walk (const char *path, const struct Options *options){
    WorkQueue wq;
    pthread_mutex_t io_mu = PTHREAD_MUTEX_INITIALIZER;
    workqueue_init(&wq);
    workqueue_push(&wq, path, 0);

    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    size_t thread_count = (cpu_count > 0) ? (size_t)cpu_count : 4;
    if (thread_count < 2) {
        thread_count = 2;
    }
    if (thread_count > 8) {
        thread_count = 8;
    }

    pthread_t *threads = calloc(thread_count, sizeof(pthread_t));
    if (!threads) {
        log_msg(&io_mu, stderr, "calloc failed: %s\n", strerror(errno));
        WorkerCtx ctx = { .wq = &wq, .options = options, .io_mu = &io_mu };
        worker_main(&ctx);
        workqueue_destroy(&wq);
        return;
    }

    WorkerCtx ctx = { .wq = &wq, .options = options, .io_mu = &io_mu };
    size_t created = 0;
    for (size_t i = 0; i < thread_count; i++) {
        int rc = pthread_create(&threads[i], NULL, worker_main, &ctx);
        if (rc != 0) {
            log_msg(&io_mu, stderr, "pthread_create failed: %s\n", strerror(rc));
            break;
        }
        created++;
    }

    if (created == 0) {
        worker_main(&ctx);
    } else {
        for (size_t i = 0; i < created; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    free(threads);
    workqueue_destroy(&wq);
}
