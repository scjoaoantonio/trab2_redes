#include "task_queue.h"


static task_t task_queue[MAX_QUEUE_SIZE];
static int queue_start = 0, queue_end = 0;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// Função para inserir uma tarefa na fila
void enqueue(task_t task) {
    pthread_mutex_lock(&queue_mutex);

    if ((queue_end + 1) % MAX_QUEUE_SIZE == queue_start) {
        fprintf(stderr, "Fila de tarefas cheia\n");
    } else {
        task_queue[queue_end] = task;
        queue_end = (queue_end + 1) % MAX_QUEUE_SIZE;
        pthread_cond_signal(&queue_cond);
    }

    pthread_mutex_unlock(&queue_mutex);
}

// Função para retirar uma tarefa da fila
task_t dequeue() {
    pthread_mutex_lock(&queue_mutex);

    while (queue_start == queue_end) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    task_t task = task_queue[queue_start];
    queue_start = (queue_start + 1) % MAX_QUEUE_SIZE;

    pthread_mutex_unlock(&queue_mutex);
    return task;
}

