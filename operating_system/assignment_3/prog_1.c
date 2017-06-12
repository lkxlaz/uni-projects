/*
 * Brief: Prog_1.c
 * Author: Angze Li
 * Date: 27 May 2017
 * Usage: ./{program name} {time quantum} {file name}
 */
#include "prog_1.h"

pthread_t threadA, threadB;
sem_t fifoRead_sem;
sem_t fifoWrite_sem;
char *fn = "/tmp/aFIFO";
FILE *fp;
char outfp[20];
int fd[2];
int TIME_QUANTUM = 4;


/*START: Implementation of QUEUE *************/

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
    item->prev = NULL;
    if (pQueue->size == 0) {
        pQueue->head = item;
        pQueue->tail = item;

    } else {
        /*adding item to the end of the queue*/
        pQueue->tail->prev = item;
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
    pQueue->head = (pQueue->head)->prev;
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
/*END: Implementation of QUEUE *************/

int initFIFO(const char *pathName, int fd[2]) {

    if(mkfifo(pathName, S_IRUSR | S_IWUSR) < 0) {
        perror("mkfifo");
        return FALSE;
    }

    // Opening a FIFO for reading blocks 
    // until some other process opens the same FIFO for writing
    if((fd[FD_READ] = open(pathName, O_RDONLY | O_NONBLOCK)) < 0) {
        perror("open FIFO");
        return FALSE;
    }
    // Opening a FIFO for writing blocks 
    // until some other process opens the same FIFO for reading
    if((fd[FD_WRITE] = open(pathName, O_WRONLY)) < 0) {
        perror("open FIFO");
        return FALSE;
    }
    return TRUE;
} 

void readFIFO(int rfd, char str[MESSLENGTH]) {
    
    int bytes;
    bytes = read(rfd, str, MESSLENGTH);
    if (bytes == -1)
        perror("read()");
}

void writeFIFO(int wfd, char *str) {
    
    if (write(wfd, str, strlen(str)+1) == -1)
        perror("write()");

}

void closeFIFO(char *pathName, int fd[2]) {

    if(unlink(pathName) < 0) {
        perror("unlink FIFO");
        exit(3);
    }

    close(fd[FD_READ]);
    close(fd[FD_WRITE]);
}

void sortByArrivalTime(Process_t pArray[], size_t size) {

    int i, j;
    Process_t temp;

    for (i=0;i<size;i++) {
        for(j=i+1;j<size;j++) {
            if (pArray[i].arrival_time > pArray[j].arrival_time) {
                temp = pArray[i];
                pArray[i] = pArray[j];
                pArray[j] = temp;
            }
        }
    }


}

// return true if all the processes in the array have been terminated
int allFinished(Process_t pArray[], size_t size) {

    int i;
    for(i = 0; i < size; i++) {
        if (pArray[i].currentState != TERMINATED) {
            return FALSE;
        }
    }

    return TRUE;
}

void roundRobin(Process_t p_array[], int size) {

    Queue *readyQueue = ConstructQueue((int)size);
    NODE *pN;
    int timeCounter = 0;

    int i;
    while (! allFinished(p_array, size)) {
        for (i=0;i<size;i++) {

            // identify newly arrived processes in the previous cpu cycle
            // and set ready state for them
            if (p_array[i].arrival_time <= timeCounter && p_array[i].currentState == UNARRIVED) {

                //state: UNARRIVED -> READY
                p_array[i].currentState = READY;
                p_array[i].waiting_time = p_array[i].cpu_remaining_time;
                pN = (NODE*) malloc(sizeof (NODE));
                pN->data = &p_array[i];
                Enqueue(readyQueue, pN);
                
                //printf("bp3: Process %d enqueued before the time %d\n", pN->data->pid, timeCounter);

            }
        }


        for (i=0;i<size;i++) {

            // get the suspended process in the last cpu cycle
            // and puth it to the end of the ready queue
            if (p_array[i].currentState == SUSPENDED) {
                pN = (NODE*) malloc(sizeof(NODE));
                p_array[i].currentState = READY;
                pN->data = &p_array[i];
                Enqueue(readyQueue, pN);
                //printf(" bp4: Process %d enqueued before the time %d\n", pN->data->pid, timeCounter);

            }
        }

        if (! isEmpty(readyQueue)) {
            pN = Dequeue(readyQueue);
            printf("P%d ", pN->data->pid);
            // state: READY -> RUNNING
            pN->data->currentState = RUNNING;

            // cpu cycle will be a whole TIME_QUANTUM
            if ((pN->data->cpu_remaining_time - TIME_QUANTUM) > 0) {
                pN->data->cpu_remaining_time -= TIME_QUANTUM;
                pN->data->currentState = SUSPENDED;
                timeCounter += TIME_QUANTUM;
                //printf("bp3: Process %d cpu rt %d\n", pN->data->pid, pN->data->cpu_remaining_time);
            }

            // cpu cycle will be the cpu remaining time of the current process
            else {
                pN->data->currentState = TERMINATED;
                timeCounter += pN->data->cpu_remaining_time;
                pN->data->cpu_remaining_time = 0;
                // calculate the turn around time and waiting time
                // turnaroundtime = finishTime - arrivalTime
                // waiting time = turnaroundTime - BurstTime
                pN->data->turnaround_time = timeCounter - pN->data->arrival_time;
                pN->data->waiting_time = pN->data->turnaround_time - pN->data->waiting_time;
                //printf("  bp4: Process %d terminated at the time %d\n", pN->data->pid, timeCounter);

            }
            //printf("  bp5: Process %d dequeued at the time %d\n", pN->data->pid, timeCounter);
            
            free(pN);
        }

        // if the ready queue is empty, keep looping.
        else
            timeCounter++; 

    }    
}

double getAvgWT(Process_t pA[], int size) {
    int sumwt, i;
    double avg_wt;

    sumwt = 0;
    for (i = 0; i < size; i++) {
        sumwt += pA[i].waiting_time;
    }
    avg_wt = (double)sumwt/size;
    return avg_wt;
}
double getAvgTAT(Process_t pA[], int size) {
    int sumtat, i;
    double avg_tat;

    sumtat = 0;
    for (i = 0; i < size; i++) {
        sumtat += pA[i].turnaround_time;
    }
    avg_tat = (double)sumtat/size;
    return avg_tat;
}

void printResults(Process_t pA[], int size) {
    int i;
    double awt, atat;

    awt = getAvgWT(pA, size);
    atat = getAvgTAT(pA, size);

    printf("\n\nPID  WT  TAT\n");
    for (i = 0; i< size; i++) {
        printf("%02d", pA[i].pid);
        printf("   %02d", pA[i].waiting_time);
        printf("   %02d", pA[i].turnaround_time);
        printf("\n");
    }

    printf("\nThe average waiting time is: %lf", awt);
    printf("\nThe average turnaround time is: %lf", atat);
}

// thread A start routine
void* threadA_routine(void *args) {

    size_t size;
    int i;
/*
    Process_t p1 = {1, 8, 10, UNARRIVED ,0, 0};
    Process_t p2 = {2, 10, 3, UNARRIVED, 0, 0};
    Process_t p3 = {3, 14, 7, UNARRIVED, 0, 0};
    Process_t p4 = {4, 9, 5, UNARRIVED, 0, 0};
    Process_t p5 = {5, 16, 4, UNARRIVED, 0, 0};
    Process_t p6 = {6, 21, 6, UNARRIVED, 0, 0};
    Process_t p7 = {7, 26, 2, UNARRIVED,0, 0};


    Process_t p_array[7] = {p1, p2, p3, p4, p5, p6, p7};
    size = sizeof(p_array) / sizeof(p_array[0]);
*/

    // get the numer of processes (the size of the array) from input
    printf("Please input the number of processes: ");
    scanf("%zd", &size);
    printf("\n");

    //create the array of process_t type.
    //note that this olny allowed on c99 compiler
    Process_t p_array[size];

    for (i = 0; i < size; i++) {
        printf("Please input the process ID, Arrival Time and Burst Time.\n");
        printf("Process %d- The Arrival Time is: ", i+1);
        scanf("%d", &p_array[i].arrival_time);
        printf("Process %d- The Burst Time is: ", i+1);
        scanf("%d", &p_array[i].cpu_remaining_time);
        printf("\n");
        p_array[i].pid = (i+1);
        p_array[i].currentState = UNARRIVED;
        p_array[i].waiting_time = 0;
        p_array[i].turnaround_time = 0;
    }

    sortByArrivalTime(p_array, size);
    printf("Gannt Chart:\n");
    roundRobin(p_array, size);
    printResults(p_array, size);
    char out[MESSLENGTH];
    sprintf(out, "The average turnaround time is: %f, The average waiting time is: %f", getAvgTAT(p_array, size), getAvgWT(p_array, size));
    if (!initFIFO(fn, fd)) {
        exit(EXIT_FAILURE);
    }
    
    sem_wait(&fifoWrite_sem);
    writeFIFO(fd[FD_WRITE], out);
    sem_post(&fifoRead_sem);
}
// thread A start routine
void* threadB_routine(void *args) {

    char in[MESSLENGTH];

    sem_wait(&fifoRead_sem);
    readFIFO(fd[FD_READ], in);
    fp = fopen(outfp, "w");
    fputs(in, fp);
    fclose(fp);
    closeFIFO(fn, fd);
    sem_post(&fifoWrite_sem);
}

int main(int argc, char *argv[]) {

    //validate arg count 
    if (argc != 3) {
        printf("INPUT ERROR: Invalid arguments\n");
        printf("usage: time quantum for the first argument. file name for the second argument.\n");
        exit(EXIT_FAILURE);
    }
    TIME_QUANTUM = atoi(argv[1]);
    strcpy(outfp, argv[2]);

    // initialize semaphores
    if (sem_init(&fifoRead_sem, 0, 0) || sem_init(&fifoWrite_sem, 0, 1)) {
        perror("Semphore init");
        exit(EXIT_FAILURE);
    }

    // create threads
    if (pthread_create(&threadA, NULL, threadA_routine, NULL) ||
        pthread_create(&threadB, NULL, threadB_routine, NULL) 
        ) {
        perror("Thread create");
        exit(EXIT_FAILURE);
    }

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    return (EXIT_SUCCESS);

/*TEST FIFO IO*********************
    char *fn = "/tmp/aFIFO";
    char *out="FIFO is fun!", in[MESSLENGTH];
    int fd[2];

    if (!initFIFO(fn, fd)) {
        exit(EXIT_FAILURE);
    }
    else
    {
        writeFIFO(fd[FD_WRITE], out);
        readFIFO(fd[FD_READ], in);
        puts(in);
        closeFIFO(fn, fd);       
    }
**********************************/
    /* TEST queue ***************************
    int i;
    Queue *pQ = ConstructQueue((int)size);
    NODE *pN;

    for (i = 0; i < size; i++) {
        pN = (NODE*) malloc(sizeof (NODE));
        pN->data = &p_array[i];
        Enqueue(pQ, pN);
    }

    while (!isEmpty(pQ)) {
        pN = Dequeue(pQ);

        printf("\nArray id: %d", p_array[0].pid);
        printf("\nNode id: %d", pN->data->pid);
        free(pN);
    }

    DestructQueue(pQ);
    *****************************************/    

}


















