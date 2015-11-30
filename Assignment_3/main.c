#include <stdio.h>
#include <pthread.h>
#include "helper.h"

#define NUM_CONSUMERS 4

void *p_thread_function(void *arg);
void *c_thread_function(void *arg);

int main()
{
  int res;
  pthread_t p_thread, c_thread[NUM_CONSUMERS];
  int num_threads = 0;

  // Create producer thread.
  res = pthread_create(&p_thread, NULL, p_thread_function, (void *)&num_threads);
  if (res != 0)
  {
    perror("Thread creation failed.\n");
    exit(EXIT_FAILURE);
  }

  // Create consumer threads.
  for (num_threads = 0; num_threads < NUM_CONSUMERS; num_threads++)
  {
    res = pthread_create(&(c_thread[num_threads]), NULL, c_thread_function, (void *)&num_threads);
    if (res != 0)
    {
      perror("Thread creation failed.\n");
      exit(EXIT_FAILURE);
    }
    //sleep(1);
  }

  printf("Done program.\n");

  return 0;
}

void *p_thread_function(void *arg) 
{
  int my_number = *(int *)arg;

  printf("This is producer %d\n", my_number);
  pthread_exit(NULL);
}

void *c_thread_function(void *arg) 
{
  int my_number = *(int *)arg;

  printf("This is consumer %d\n", my_number);
  pthread_exit(NULL);
}
