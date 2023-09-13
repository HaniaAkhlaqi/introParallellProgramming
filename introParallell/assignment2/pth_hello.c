/* File:  
 *    pth_hello.c
 *
 * Purpose:
 *    Illustrate basic use of pthreads:  create some threads,
 *    each of which prints a message.
 *
 * Input:
 *    none
 * Output:
 *    message from each thread
 *
 * Compile:  gcc -g -Wall -o pth_hello pth_hello.c -lpthread
 * Usage:    ./pth_hello <thread_count>
 *
 * IPP:   Section 4.2 (p. 153 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS  100

long thread_count; // shared between threads

/*-------------------------------------------------------------------*/
void *hello(void *args) {
  long t = (long)args;
  printf("Hello from thread %ld out of %ld\n", t, thread_count);
  return NULL;
} /* Hello */

/*-------------------------------------------------------------------*/
void usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
   fprintf(stderr, "0 < number of threads <= %d\n", MAX_THREADS);
   exit(0);
}  /* Usage */

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
  long thread;
  pthread_t* th;
  
  /* Get number of threads from command line */
  if (argc != 2) usage(argv[0]);
  thread_count = strtol(argv[1], NULL, 10);  
  if (thread_count <= 0 || thread_count > MAX_THREADS) usage(argv[0]);

  th = malloc (thread_count * sizeof(pthread_t));

  /* During the lab, we modified the program so that the hello message
     from the main thread is printed _in the middle_ of the messages
     from spawned threads. */
  for (thread = 0; thread < thread_count / 2; thread++)
    pthread_create(&th[thread], NULL, hello, (void *) thread);

  for (thread = 0; thread < thread_count / 2; thread++)
    pthread_join(th[thread], NULL);

  printf("Hello from the main thread\n");

  for ( ; thread < thread_count; thread++)
    pthread_create(&th[thread], NULL, hello, (void *) thread);

  for (thread = thread_count / 2; thread < thread_count; thread++)
    pthread_join(th[thread], NULL);

  free(th);
  printf("Goodbye from the main thread\n");
  return 0;
}  /* main */
