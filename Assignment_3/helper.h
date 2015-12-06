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
#define QUEUE_SZ  100

// Structs.
// Process information created by the producer.
struct _process_info {
  int pid;
  int static_prio;      // Static priority: The default static priority is 120, even for real-time processes. Priorities 0-99 are for real-time processes and 100-139 are for normal processes. Lower values represent higher priority levels.
  int prio;             // Dynamic priority: the dynamic priority is calculated as a function of the static priority and the task’s interactivity.
  int expected_run;     // Expected execution time: simply an integer between 5 and 50 (seconds).
  int time_slice;
  int accu_time_slice;  // Accumulated time slice.
  int last_cpu;         // The CPU (thread) that the process last ran.
  char sched[13];       // SCHED_FIFO/SCHED_RR/SCHED_NORMAL
};

// A queue that holds processes.
struct _queue {
  int in;
  int out;
  int size;
  struct _process_info *processes[QUEUE_SZ];
};

// Initialize the queue with processes.
void init_queue(struct _queue *queue);

// Adds a process to the queue of processes.
void append(struct _process_info *process, struct _queue *queue);

// Consumes the highest priority process from the queue.
struct _process_info *take(struct _queue *queue);

// Re-orders a queue according to priority.
void reorder(struct _queue *queue);

#endif
