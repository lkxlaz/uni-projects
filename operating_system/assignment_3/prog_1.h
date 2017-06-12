/*
 * Brief: Prog_1.h
 * Author: Angze Li
 * Date: 27 May 2017
 * Usage: ./{program name} {time quantum} {file name}
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MESSLENGTH 80
#define FD_READ 0
#define FD_WRITE 1

/* Custom Data Types */
enum status {
    UNARRIVED,     // arrived in ready queue
    READY,   // in ready queue
    RUNNING,   // on cpu
    SUSPENDED, // put back to the ready queue
    TERMINATED  // finished
};

typedef struct {
    int pid;
    int arrival_time;
    int cpu_remaining_time;
    enum status currentState;
    int waiting_time;
    int turnaround_time;
} Process_t;

typedef struct Node_t {
    Process_t *data;
    struct Node_t *prev;
} NODE;

typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

/*FUNCTION PROTOTYPES*/
Queue *ConstructQueue(int limit);
void DestructQueue(Queue *queue);
int Enqueue(Queue *pQueue, NODE *item);
NODE *Dequeue(Queue *pQueue);
int isEmpty(Queue* pQueue);
int initFIFO(const char *fifoName, int fd[2]);
void readFIFO(int rfd, char *str);
void writeFIFO(int wfd, char *str);
void closeFIFO(char *pathName, int fd[2]);

