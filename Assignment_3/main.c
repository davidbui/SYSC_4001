#include <stdio.h>
#include <pthread.h>
#include "helper.h"

#define NUM_CPU   4
#define NUM_RQ    3

// Thread functions.
void *producer_t(void *arg); // Producer thread.
void *consumer_t(void *arg); // Consumer thread that emulates a cpu.
void *balancer_t(void *arg); // Load balancer thread periodically checks the number
                             // of processes in all the run queues and balance the run queue.

// Functions.
struct _process_info *create_process_info();

// Global Variables.
struct _queue queue[NUM_CPU][NUM_RQ];   // There are 4 CPUs, each with 3 RQ's
pthread_mutex_t mutex[NUM_CPU][NUM_RQ]; // Protects each queue for each cpu.


int main()
{
  // Variables.
  int res;
  pthread_t p_thread, c_thread[NUM_CPU];
  int num_threads = 0;

  // Create mutexes.
  for (int i=0; i<NUM_CPU; i++) {
    for (int j=0; j<NUM_RQ; j++) {
      res = pthread_mutex_init(&(mutex[i][j]), NULL);
      if (res != 0) {
        perror("Mutex Initialization failed.");
      }
    }
  }

  // Create producer thread.
  res = pthread_create(&p_thread, NULL, producer_t, (void *)&num_threads);
  if (res != 0)
  {
    perror("Thread creation failed.\n");
    exit(EXIT_FAILURE);
  }

  // Fill the cpu run queues

  // Create cpu threads.
  for (num_threads = 0; num_threads < NUM_CPU; num_threads++)
  {
    res = pthread_create(&(c_thread[num_threads]), NULL, consumer_t, (void *)&num_threads);
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

void *producer_t(void *arg) 
{
  int my_number = *(int *)arg;
  int in = 0;
  
  printf("This is producer %d\n", my_number);

  while(1)
  {
    fprintf(stderr, "Creating process info.\n");
    struct _process_info *p = create_process_info();

    fprintf(stderr, "Putting process info item into buffer %d.\n", in);


  }

  pthread_exit(NULL);
}

void *consumer_t(void *arg) 
{
  int my_number = *(int *)arg;

  printf("This is consumer %d\n", my_number);
  pthread_exit(NULL);
}

void *balancer_t(void *arg) 
{
  int my_number = *(int *)arg;

  printf("This is the balancer %d\n", my_number);
  pthread_exit(NULL);
}

struct _process_info *create_process_info()
{
  struct _process_info *info = (struct _process_info*)malloc(sizeof(struct _process_info));
  info->pid = 1000;
  info->static_prio = 1;
  info->prio = 2;
  info->expected_run = 5;
  info->time_slice = 2;
  info->accu_time_slice = 4;
  info->last_cpu = 0;

  return info;
}
