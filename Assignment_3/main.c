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
int calc_time_slice(int SP); // Calculates a time slice of a given process based on static priority.
int DP();

// Global Variables.
struct _queue queue[NUM_CPU][NUM_RQ];   // There are 4 CPUs, each with 3 RQ's
pthread_mutex_t queue_lock[NUM_CPU][NUM_RQ]; // Protects each queue for each cpu.
pthread_mutex_t var_lock; // Protects each queue for each cpu.
int num_processes; // Stores the total number of processes that have not been finished.
int bonus[10] = {0,5,10,10,0,0,5,10,10,0};
int main()
{
  // Variables.
  int res;
  pthread_t p_thread, c_thread[NUM_CPU], b_thread;
  void *thread_result;
  int num_threads = 0;

  // Create mutexes and initialize the queues for each cpu.
  for (int i=0; i<NUM_CPU; i++) {
    for (int j=0; j<NUM_RQ; j++) {
      // Initialize mutexes.
      res = pthread_mutex_init(&(queue_lock[i][j]), NULL);
      if (res != 0) {
        perror("Mutex Initialization failed.");
        exit(EXIT_FAILURE);
      }
      // Initialize queues.
      init_queue(&queue[i][j]);
    }
  }

  res = pthread_mutex_init(&(var_lock), NULL);
  if (res != 0) {
    perror("Mutex Initialization failed.");
    exit(EXIT_FAILURE);
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


    // Create balancer thread.
  res = pthread_create(&b_thread, NULL, balancer_t, NULL);
  if (res != 0)
  {
    perror("Thread creation failed.\n");
    exit(EXIT_FAILURE);
  }

  // Create cpu threads.
  for (num_threads = 0; num_threads < NUM_CPU; num_threads++)
  {
    res = pthread_create(&(c_thread[num_threads]), NULL, consumer_t, (void *)&num_threads);
    if (res != 0)
    {
      perror("Thread creation failed.\n");
      exit(EXIT_FAILURE);
    }
    sleep(1);
  }

  res = pthread_join(b_thread, &thread_result);
  if (res != 0) {
      perror("Thread join failed.");
      exit(EXIT_FAILURE);
  }

  printf("Balancer Thread joined.\n");

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
  num_processes = atoi(buff);

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
      pthread_mutex_lock(&queue_lock[i%4][1]);
      append(p_temp, &queue[i%4][1]);
      pthread_mutex_unlock(&queue_lock[i%4][1]);
    } else {
      pthread_mutex_lock(&queue_lock[i%4][0]);
      append(p_temp, &queue[i%4][0]);
      pthread_mutex_unlock(&queue_lock[i%4][0]);
    }
  } // End for loop.
  pthread_exit(NULL);
} // End producer thread.

void *consumer_t(void *arg) 
{
  int num_cpu = *(int *)arg;
  int current_rq = 0;

  printf("CPU %d started.\n", num_cpu);
  //num_cpu--;
  struct _process_info *cur_process;
  int num_processes_rq[3] = {0}; // number of proceses per RQ0/1/2
  char sched[13];     
  int t1,t2;
  int prio2;
  // Loop while there are at least 3 processes.
  while (1) {
    // Consume tasks in all RQ01/2.
    while (current_rq < NUM_RQ) {
      // Consume processes in the higest RQ first and then move onto the next(RQ0 -> RQ1 -> RQ2).
      while ((cur_process = take(&queue[num_cpu][current_rq])) != NULL) {
        // Consume the task.
        pthread_mutex_lock(&queue_lock[num_cpu][current_rq]);

        // RQ0 = SCHED_FIFO/RR
        if (current_rq == 0) {
          // FIFO scheme processes are consumed until it is finished.
          if (strcmp(cur_process->sched, "SCHED_FIFO") == 0) {
            printf("CPU %d: Process PID %d (SCHED_FIFO) is running, time slice allocated = %dms.\n", num_cpu, cur_process->pid, cur_process->expected_run);
            usleep((cur_process->expected_run)*1000);
            free(cur_process); // Free the resources.
            printf("CPU %d: Process with PID %d finished!\n", num_cpu, cur_process->pid);
            pthread_mutex_lock(&var_lock);
            num_processes--;
            printf("CPU %d: There are %d processes left!\n", num_cpu, num_processes);
            pthread_mutex_unlock(&var_lock);
          } else {
            // SCHED_RR.
            printf("CPU %d: Process PID %d (SCHED_RR) is running, time slice allocated = %dms, Execution time left = %d.\n", \
                                num_cpu, cur_process->pid, cur_process->time_slice, cur_process->expected_run - cur_process->accu_time_slice);
            usleep((cur_process->time_slice)*1000);
            cur_process->accu_time_slice+=cur_process->time_slice;

            // Process has not yet completed, add back into RQ.
            if (cur_process->accu_time_slice < cur_process->expected_run) {
              append(cur_process, &queue[num_cpu][current_rq]);
              printf("CPU %d: Adding process with PID %d back to RQ%d.\n", num_cpu, cur_process->pid, current_rq);
            } else {
              printf("CPU %d: Process with PID %d finished!\n", num_cpu, cur_process->pid);
              free(cur_process); // Free the resources.
              pthread_mutex_lock(&var_lock);
              num_processes--;
              printf("CPU %d: There are %d processes left!\n", num_cpu, num_processes);
              pthread_mutex_unlock(&var_lock);
            }

          }
        } else {
          // SCHED_NORMAL.
          printf("CPU %d: Process PID %d (SCHED_NORMAL) is running, time slice allocated = %dms, Execution time left = %d.\n", \
                                num_cpu, cur_process->pid, cur_process->time_slice, cur_process->expected_run - cur_process->accu_time_slice);

          usleep((cur_process->time_slice)*1000);
          
          cur_process->accu_time_slice+=(cur_process->time_slice);
          // Process has not yet completed, add back into RQ.
          if (cur_process->accu_time_slice < cur_process->expected_run) {
            // Update the dynamic priority.
            prio2 = DP(cur_process->prio);
            if (cur_process->prio != prio2) {
              printf("CPU %d: Process PID %d (SCHED NORMAL IS CHANGED!), new priority = %d, previous priority = %d.\n", \
                      num_cpu, cur_process->pid, prio2, cur_process->prio);
              cur_process->prio = prio2;
            }

            // If priority is >= 130, place into RQ2.
            if (cur_process->prio >= 130) {
              append(cur_process, &queue[num_cpu][2]);
              printf("CPU %d: Process PID %d placed into RQ2\n", num_cpu, cur_process->pid);
            } else {
              append(cur_process, &queue[num_cpu][1]);
              printf("CPU %d: Process PID %d placed into RQ1\n", num_cpu, cur_process->pid);
            }
          } else {
            printf("CPU %d: Process with PID %d finished!\n", num_cpu, cur_process->pid);
            free(cur_process); // Free the resources.
            pthread_mutex_lock(&var_lock);
            num_processes--;
            printf("CPU %d: There are %d processes left!\n", num_cpu, num_processes);
            pthread_mutex_unlock(&var_lock);
          }

        }

        // The load balancer could have put something into a higher priority queue and the cpu is currently in a lower priority queue.
        // Move back if so.

        // Update number of processes per rq0/1/2.
        for (int i=0; i<NUM_RQ; i++) {
          num_processes_rq[i] = (queue[num_cpu][i].in - queue[num_cpu][i].out); 
        }

        // Roll back to higher priority queue.
        for (int i=NUM_RQ-2; i>=0; i--) {
          // There is a process in RQ1/0 and we may be in a lower priority queue.
          if ((num_processes_rq[i] > 0) && (current_rq > i)) {
            current_rq = i;
          }
        }

        pthread_mutex_unlock(&queue_lock[num_cpu][current_rq]);
      } // End while loop cause no more proesses in current RQ.
      // No more processes in current RQ, move onto next lower priority queue.
      current_rq++;
    } // End while loop cause finished all proesses in queue.
    // No more proesses in any RQ.. BUT, proesses from another CPU RQ could be transfered here by the load balancer..
  } // End main while loop.

  printf("CPU %d IS DONE ALL PROCESSSES\n", num_cpu);
  pthread_exit(NULL);
}

void *balancer_t(void *arg) 
{
  int num_processes_rq[NUM_CPU] = {0};
  printf("Blancer starting.\n");
  int balance = 0;
  while(1) {
    pthread_mutex_lock(&var_lock);
    if (num_processes <= 0) {
      printf ("Blancer: there are no processes left!\n");
      break;
    }
    pthread_mutex_unlock(&var_lock);
    printf("Balanced cpu queues.\n");

    // Update number of processes per rq0/1/2.
    for (int j=0; j<NUM_CPU; j++) {
      for (int i=0; i<NUM_RQ; i++) {
        num_processes_rq[i] = (queue[j][i].in - queue[j][i].out); 
      }
    }

    // Move process into another cpu.
    for (int i=0; i<NUM_CPU; i++) {
      while (balance) {
        num_processes_rq[i] = num_processes_rq[1];
      }
    }
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
  info->time_slice = calc_time_slice(info->static_prio);
  info->accu_time_slice = 0;
  info->last_cpu = pid % 4;
  info->sleep_avg = 0;

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

int calc_time_slice(int SP)
{
  if (SP < 120) 
    return (140-SP)*20;
  else
    return (140-SP)*5;
}

int DP(int prev_prio)
{
  int x = rand() % 10;
  int ret;
  if (bonus[x] == 0)
    return prev_prio;
  else if (bonus[x] == 10) {
    if ((prev_prio+5)<140)
      return prev_prio+5;
    else return 139;
  }
  
  if ((prev_prio-5)<100)
    return 100;
  return prev_prio-5;
}

