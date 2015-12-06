#include <stdio.h>
#include <pthread.h>
#include "helper.h"
#include <time.h>

#define NUM_CPU       4
#define NUM_RQ        3
#define NUM_PROCESSES 20

// Threads.
void *producer_t(void *arg); // Producer thread.
void *consumer_t(void *arg); // Consumer thread that emulates a cpu.
void *balancer_t(void *arg); // Load balancer thread periodically checks the number
                             // of processes in all the run queues and balance the run queue.

// Functions.
struct _process_info *create_process_info(char *line, int pid); // Creates a process.
void print_current_processes(void); // Prints all the current processes in each of the queues in each CPU.

// Global Variables.
struct _queue queue[NUM_CPU][NUM_RQ];   // There are 4 CPUs, each with 3 RQ's
pthread_mutex_t mutex[NUM_CPU][NUM_RQ]; // Protects each queue for each cpu.


int main()
{
  // Variables.
  int res;
  pthread_t p_thread, c_thread[NUM_CPU];
  void *thread_result;
  int num_threads = 0;

  // Create mutexes and initialize the queues for each cpu.
  for (int i=0; i<NUM_CPU; i++) {
    for (int j=0; j<NUM_RQ; j++) {
      // Initialize mutexes.
      res = pthread_mutex_init(&(mutex[i][j]), NULL);
      if (res != 0) {
        perror("Mutex Initialization failed.");
        exit(EXIT_FAILURE);
      }
      // Initialize queues.
      init_queue(&queue[i][j]);
    }
  }

  // Create producer thread.
  res = pthread_create(&p_thread, NULL, producer_t, NULL);
  if (res != 0)
  {
    perror("Thread creation failed.\n");
    exit(EXIT_FAILURE);
  }

  printf("\nWaiting for producer thread to finish...\n");
  res = pthread_join(p_thread, &thread_result);
  if (res != 0) {
      perror("Thread join failed.");
      exit(EXIT_FAILURE);
  }
  printf("Producer Thread joined.\n");

  printf("\nInitial process distribution.\n");
  print_current_processes();

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

  printf("\nWaiting for consumer thread to finish...\n");
  res = pthread_join(p_thread, &thread_result);
  if (res != 0) {
      perror("Thread join failed.");
      exit(EXIT_FAILURE);
  }
  printf("Consumer Thread joined.\n");

  printf("Done program.\n");

  return 0;
}

void *producer_t(void *arg) 
{
  printf("This is producer.\n");

  printf("Generating process infomation from text file.\n");
  FILE *fp;
  char buff[255];

  fp = fopen("processes.txt", "r");

  if (fp == NULL) {
    fprintf(stderr, "Producer: Error opening file.\n");
    exit(EXIT_FAILURE);
  }

  fgets(buff, 100, fp);
  int num_processes = atoi(buff);

  // Check if there is an evenly distributible number of processes.
  if (num_processes % NUM_CPU != 0) {
    fprintf(stderr, "Producer: Invalid number of processes.\n");
    exit(EXIT_FAILURE);
  }

  printf("Number of processes = %d\n", num_processes);

  int pid = 0;
  struct _process_info* p_temp;

  // Begin reading in processes and placing them into the CPU queues.
  for (int i=1; i<=num_processes; i++) {
    if (fgets(buff, 100, fp) == NULL) {
      fprintf(stderr, "Producer: EOF reached (Missing lines).\n");
      exit(EXIT_FAILURE);
    }

    // Create process.
    p_temp = create_process_info(buff, i);

    // Place process into correct queue.
    if (strcmp(p_temp->sched, "SCHED_NORMAL") == 0) {
      append(p_temp, &queue[i%4][1]);
    } else {
      append(p_temp, &queue[i%4][0]);
    }
  } // End for loop.
  pthread_exit(NULL);
} // End producer thread.

void *consumer_t(void *arg) 
{
  int num_cpu = *(int *)arg;

  printf("This is cpu %d\n", num_cpu);

  //while ( )
  pthread_exit(NULL);
}

void *balancer_t(void *arg) 
{
  int my_number = *(int *)arg;

  printf("This is the balancer %d\n", my_number);

  while(1) {

    printf("Balanced cpu queues.\n");
    sleep(1);
  }
  pthread_exit(NULL);
}

struct _process_info *create_process_info(char *line, int pid)
{
  char *token;
  const char delimiter[] = ",";
  struct _process_info *info = (struct _process_info*)malloc(sizeof(struct _process_info));

  token = strtok(line, delimiter); // sched type.
  strcpy(info->sched, token);
  token = strtok(NULL, delimiter); // static_prio.
  info->static_prio = atoi(token);
  token = strtok(NULL, delimiter); // expected_run.
  info->expected_run = atoi(token);
  info->pid = pid;
  info->prio = info->static_prio;
  info->time_slice = 0;
  info->accu_time_slice = 0;
  info->last_cpu = pid % 4;

  return info;
}

void print_current_processes(void)
{
  struct _process_info *process;
  printf("%-3s | %-3s | %-8s | %-11s | %-4s | %-12s | %-10s | %-15s | %-12s\n",\
         "PID", "CPU", "last_cpu", "static_prio", "prio", "expected_run", "time_slice", "accu_time_slice", "sched_type");

  for (int i=0; i<NUM_CPU; i++) {
    for (int j=0; j<NUM_RQ; j++) {
      for (int k=queue[i][j].out; k<queue[i][j].in; k++) {
        process = queue[i][j].processes[k];
        printf("%-3d | %-3d | %-8d | %-11d | %-4d | %-12d | %-10d | %-15d | %-12s\n",\
               process->pid, i, process->last_cpu, process->static_prio, process->prio, process->expected_run, \
               process->time_slice, process->accu_time_slice, process->sched);
      }
    }
  }

}
