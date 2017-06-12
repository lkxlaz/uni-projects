/*
 * Brief: Prog_2.c
 * Author: Angze Li
 * Date: 27 May 2017
 * Usage: ./{program name} {frame number} 
 */
#include "prog_2.h"

/*START: Implementation of QUEUE *************/

/*flag to indicate if the SIGINT received*/
volatile int interrupted_flag;

Queue *ConstructQueue(int limit) {
    Queue *queue = (Queue*) malloc(sizeof (Queue));
    if (queue == NULL) {
        return NULL;
    }
    if (limit <= 0) {
        limit = 65535;
    }
    queue->limit = limit;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void DestructQueue(Queue *queue) {
    NODE *pN;
    while (!isEmpty(queue)) {
        pN = Dequeue(queue);
        free(pN);
    }
    free(queue);
}

int Enqueue(Queue *pQueue, NODE *item) {
    /* Bad parameter */
    if ((pQueue == NULL) || (item == NULL)) {
        return FALSE;
    }
    // if(pQueue->limit != 0)
    if (pQueue->size >= pQueue->limit) {
        return FALSE;
    }
    /*the queue is empty*/
    item->next = NULL;
    if (pQueue->size == 0) {
        pQueue->head = item;
        pQueue->tail = item;

    } else {
        /*adding item to the end of the queue*/
        pQueue->tail->next = item;
        pQueue->tail = item;
    }
    pQueue->size++;
    return TRUE;
}

NODE * Dequeue(Queue *pQueue) {
    /*the queue is empty or bad param*/
    NODE *item;
    if (isEmpty(pQueue))
        return NULL;
    item = pQueue->head;
    pQueue->head = (pQueue->head)->next;
    pQueue->size--;
    return item;
}

int isEmpty(Queue* pQueue) {
    if (pQueue == NULL) {
        return FALSE;
    }
    if (pQueue->size == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int isFull(Queue* pQueue) {
	if (pQueue == NULL)
		return FALSE;
	if (pQueue->size == pQueue->limit)
		return TRUE;
	else
		return FALSE;
}

int isInQueue(Queue *pQueue, NODE *item) {
	if (pQueue == NULL || item == NULL)
		return FALSE;
	NODE *p = pQueue->head;
	while(p != NULL) {
		if (p->data->pno == item->data->pno)
			return TRUE;
		p = p->next;
	}
	return FALSE;
}
/*END: Implementation of QUEUE *************/

void signal_handler() {
    // set the flag
    interrupted_flag = TRUE;
}

int getPageSize(const char str[], int size) {
    int pgSize = 0;
    int i;

    for (i = 0; i < size-1; i++) {
        if (str[i] != ',')
            pgSize++;
    }
    return pgSize;
}

int getPagefaultNumber(Page_t pgArray[], int size) {
	int number = 0;
	int i;
	for (i = 0; i < size; i++) {
		if (pgArray[i].page_fault == TRUE)
			number++;
	}
	return number;
}

void printQueue(Queue *pQueue) {
    int i, j, frameSize;
    NODE *p;
    
    frameSize = pQueue->limit;
    p = pQueue->head;
    i = 0;

    while (p != NULL) {
        printf(" %d   ", p->data->pno);
        p = p->next;
        i++;
    }
    if (i < frameSize) {
        // leftover elements will be displayed as -1
        // indicating that the frame page os empty
        j = frameSize - i;
        while (j--) {
            printf("-1   ");
        }
    }
    printf("    ");
    
    
}

int main(int argc, char *argv[]) {

	int frameSize, pageSize, refstrLength, i;
	const char refstr[] = "7,0,1,2,0,3,0,4,2,3,0,3,0,3,2,1,2,0,1,7,0,1,7,5";
    //const char refstr[] = "7,0,1,2,0,3,0,4,2,3,0,3,0,3,2,1,2,0,1,7,0,1";
	Queue *fQueue;

    //validate arg count 
    if (argc != 2) {
        printf("INPUT ERROR: Invalid arguments\n");
        printf("usage: the first argument should be the number of the frames.\n");
        exit(EXIT_FAILURE);
    }
    frameSize = atoi(argv[1]);
    

    
    pageSize = getPageSize(refstr, sizeof(refstr)/sizeof(refstr[0]));
    Page_t pageArray[pageSize];
    // convert the reference string to page_t type array
    refstrLength = (int)strlen(refstr);
    for (i = 0; i < refstrLength; i++) {
    	if (refstr[i] != ',') {
    		pageArray[i/2].pno = (int)(refstr[i] - '0');
    		pageArray[i/2].page_fault = FALSE;
    	}
    }

    // create the queue with frame size
    fQueue = ConstructQueue(frameSize);

    // iterate through the page array
    // identify the page fault
    printf("F1 | F2 | F3 | F4 |    P.no | Page fault?\n");
    printf("-------------------    ----   -----------\n\n");
    NODE *pN;
    for (i = 0; i < pageSize; i++) {
    	pN = (NODE *)malloc(sizeof(NODE));
    	pN->data = &pageArray[i];
    	if (! isInQueue(fQueue, pN)) {
    		pageArray[i].page_fault = TRUE;
    		if (isFull(fQueue)) {
    			Dequeue(fQueue);
    		}
    		Enqueue(fQueue, pN);
    	}
    	else
    		free(pN);
        // print the state of the current frame
        printQueue(fQueue);
        printf("%d         ", pageArray[i].pno);
        pageArray[i].page_fault == TRUE ? printf("Y\n") : printf("N\n");
    }
    
    // initialize and register user defined signal handler
    // initialize the flag
    interrupted_flag = FALSE;
    signal(SIGINT, signal_handler);
    printf("Please press Ctrl + C to show the total page fault number\n");
    
    // polling for the interrupted flag
    while(interrupted_flag != TRUE){}

    int pgfaultNumber;
    pgfaultNumber = getPagefaultNumber(pageArray, pageSize);
    printf("-----------------------------------------\n\n");
    printf("The page fault number is: %d\n", pgfaultNumber);

    //free memory for the queue
    DestructQueue(fQueue);

    return (EXIT_SUCCESS);

}


