/*
 * Brief: Prog_2.h
 * Author: Angze Li
 * Date: 27 May 2017
 * Usage: ./{program name} {frame number} 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct {
	int pno;
	int page_fault; /*binary state variable. TRUE or FALSE*/
} Page_t;

typedef struct Node_t {
    Page_t *data;
    struct Node_t *next;
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
int isFull(Queue* pQueue); 
