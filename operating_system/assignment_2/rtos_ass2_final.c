/*
 * title : real time operating system - Assignment 2
 * author: Angze Li - 99178333
 * date  : 1st May 2017
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFFER_SIZE 50

typedef struct 
{
	char buf[BUFFER_SIZE];
	int pipefd[2];
	FILE* fp0;
	FILE* fp1;
	sem_t* pipeRead_sem;
	sem_t* pipeWrite_sem;
	sem_t* bufAvailable_sem;

} ThreadData_t;

pthread_t threadA, threadB, threadC;

int content_reached;
int eof_reached;

/* brief: read each line at a time from the given file 
 * return: upon successful, return 1. or return 0 when EOF/error occured
 */
int read_line (FILE* fp, char buf[]) {

	if (fgets(buf, 50, fp) != NULL)
		return 1;

	// end of file check
	if (feof(fp)) {
		puts("End of file reached");
		eof_reached = 1;
		return 0;
	}

	// other errors check
	if (ferror(fp)) {
		puts("Error indicator set");
		return 0;
	}
}

/* brief: write a line into the given file
 * return: none
 */
void write_line(FILE* fp, char buf[]) {

	fputs(buf, fp);

	printf("write line: %s", buf);
}

// detects end of header. if end of header rached, set global bool content_reached = true
void detect_contents(char str[]) {

	if ( strstr(str, "end_header")  )
		content_reached = 1;
}

// create a pipe with given file descriptors
// return: if successful, return 0. or return -1 and print out error message.
int pipe_create(int fd[2]) {

	if (pipe(fd) == -1) {
        perror("pipe error");
        return -1;
    }
    else 
		return 0;

}

// write in pipe with given string
void pipe_in(int fd[2], char str[]) {

	// write
	write(fd[1], str, BUFFER_SIZE);

	printf("pipe in: %s", str);
	memset(str, '\0', strlen(str));
	// close the fd once finished
	close(fd[1]);
}

// read from pipe
void pipe_out(int fd[2], char str[]) {

	memset(str, '\0', strlen(str));

	// read
	read(fd[0], str, BUFFER_SIZE);
	printf("pipe out: %s", str);

	// close 
	close(fd[0]);
}	

// terminate all threads
void terminateAll() {

	pthread_cancel(threadA);
	pthread_cancel(threadB);
	pthread_cancel(threadC);
}

// start_routine for thread A
void* threadA_routine (void *args) {

	// local variable
	char buf[BUFFER_SIZE];
	int rc;

	ThreadData_t * this_data = (ThreadData_t *)args;

	for (;;) {
	// block until pipe write semaphore reached
	sem_wait(this_data->pipeWrite_sem);

	printf("......................................\n");
	// read a line from data.txt
	// if eof reached, then thread A B C will stop
	rc = read_line(this_data->fp0, buf);
	if ((rc == 0) && (eof_reached == 1))
		terminateAll();

	// create a pipe 
	pipe_create(this_data->pipefd);

	// write the line to pipe
	pipe_in(this_data->pipefd, buf);

	printf("thread A completed\n");

	// post the pipe read signal
	sem_post(this_data->pipeRead_sem);

	}

}

// start_routine for thread B
void* threadB_routine (void *args) {

	ThreadData_t * this_data = (ThreadData_t *)args;

	for (;;) {
	// block until pipe read semphore reached
	sem_wait(this_data->pipeRead_sem);

	// read the line of pipe and store the data to an array buffer
	pipe_out(this_data->pipefd, this_data->buf);

	printf("thread B completed\n");


	// post the buffer available signal
	sem_post(this_data->bufAvailable_sem);

	}
	
}

// start_routine for thread C
void* threadC_routine (void *args) {

	ThreadData_t * this_data = (ThreadData_t *)args;

	for (;;) {
	// bolck until buffer available semaphore reached
	sem_wait(this_data->bufAvailable_sem);

	// if the line of data is detected to be header
	// then discard
	if (content_reached)
		write_line(this_data->fp1, this_data->buf);
	else
		printf("line discarded\n");
	//printf("detecting content: %s", Buf);
	detect_contents(this_data->buf);
	printf("thread C completed\n");
	// post pipe write semaphore
	sem_post(this_data->pipeWrite_sem);
	}
	
}


int main() {

	// define local variables
	ThreadData_t thread_data;
	sem_t read, write, justify;
	FILE* fp0;
	FILE* fp1;

	// set globals
	content_reached = 0;
	eof_reached = 0;

	// initialize semaphores
	if (sem_init(&read, 0, 0) || sem_init(&write, 0, 1) || sem_init(&justify, 0, 0)) {
		perror("Semphore init");
		exit(EXIT_FAILURE);
	}
	thread_data.pipeRead_sem = &read;
	thread_data.pipeWrite_sem = &write;
	thread_data.bufAvailable_sem = &justify;

	// intialize file I/O
	thread_data.fp0 = fopen("data.txt", "r");
	thread_data.fp1 = fopen("src.txt", "w");
	if (! (thread_data.fp0 && thread_data.fp1)) {
		perror("File opening failed");
		exit(EXIT_FAILURE);
	}

	// initialize threads
	if (pthread_create(&threadA, NULL, threadA_routine, (void *)&thread_data) ||
		pthread_create(&threadB, NULL, threadB_routine, (void *)&thread_data) ||
		pthread_create(&threadC, NULL, threadC_routine, (void *)&thread_data)
		) {
		perror("Thread create");
		exit(EXIT_FAILURE);
	}

	pthread_join(threadA, NULL);
	pthread_join(threadB, NULL);
	pthread_join(threadC, NULL);

	// close up files
	fclose(thread_data.fp0);
	fclose(thread_data.fp1);

	printf("Main completed\n");

	return 0;

/* Test code below ***********

	char tmpbuf[50], tmpbuf1[50];
	read_line(fp0, tmpbuf);
	printf("line read from data.txt: %s\n", tmpbuf);
	if ( ! pipe_create(pipefd)) {
		printf("pipe created\n");
		pipe_in(pipefd, tmpbuf);
		pipe_out(pipefd, Buf);
		printf("data read from pipe: %s\n", Buf);
		write_line(fp1, Buf);

	}
*/



/* test pipe I/O
	char buf[100];
	if (! pipe_create(pipefd)) {
		printf("pipe created\n");
		printf("%d, %d\n", pipefd[0], pipefd[1]);
		pipe_in(pipefd, "hello,world");
		pipe_out(pipefd, Buf);
		puts(Buf);

	}
*/


}



