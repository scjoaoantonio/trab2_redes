#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define MAX_QUEUE_SIZE 10

// Estrutura da tarefa
typedef struct {
    int socket_fd;
} task_t;

// Funções relacionadas à fila de tarefas
void enqueue(task_t task);
task_t dequeue();

#endif // TASK_QUEUE_H
