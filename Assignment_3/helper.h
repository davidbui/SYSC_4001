#ifndef HELPER_H
#define HELPER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h> // stat().
#include <fcntl.h>    // open().

// Constants.
#define CBUFFER_SZ  100
#define BUFSIZE     128
#define TEXT_SZ     25

#define SHM_ID      2981
#define SEM_ID_S    2982
#define SEM_ID_N    2983
#define SEM_ID_E    2984

// Structs.
// Process information created by the producer.
struct _p_info {
  int pid;
  int static_prio;      // Static priority: The default static priority is 120, even for real-time processes. Priorities 0-99 are for real-time processes and 100-139 are for normal processes. Lower values represent higher priority levels.
  int prio;             // Dynamic priority: the dynamic priority is calculated as a function of the static priority and the task’s interactivity.
  int expected_run;     // Expected execution time: simply an integer between 5 and 50 (seconds).
  int time_slice;
  int accu_time_slice;  // Accumulated time slice.
  int last_cpu;         // The CPU (thread) that the process last ran.
} p_info;

// Global variables.
int sem_id_s; // Provides mutual exclusion on buffer access.
int sem_id_n; // Synchronize producer and consumer on the number of consumable items.
int sem_id_e; // Synchronize producer and consumer on the number of empty spaces.

// Semaphore function prototypes.
int set_semvalue(int sem_id, int init_value);
void del_semvalue(int sem_id);
int p_wait(int sem_id);
int p_signal(int sem_id);

#endif
