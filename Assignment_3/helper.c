#include "helper.h"

// Initialize the queue with processes.
void init_queue(struct _queue *queue)
{
  queue->in = 0;
  queue->out = 0;
  queue->size = QUEUE_SZ;
}

// Adds a process to the queue of processes, and then re-orders them according to priority. (ASSUME QUEUE WILL NEVER BE FULL)
void append(struct _process_info *process, struct _queue *queue)
{
  queue->processes[queue->in] = process;
  queue->in = (queue->in +1) % queue->size;
  reorder(queue);
}

// Consumes the highest priority process from the queue. Returns null if empty.
struct _process_info *take(struct _queue *queue)
{
  if (queue->in == queue->out) {
    return NULL;
  } else {
    struct _process_info *p = queue->processes[queue->out];
    queue->out = (queue->out+1)%queue->size;
    return p;
 }
}

// Reorders a queue with the highest priorty placed in the front of the queue.
void reorder(struct _queue *queue)
{
  struct _process_info *temp_process;
  // Simple bubble sort of prio.
  for (int i=(queue->out+1); i<queue->in; i++) {
    for (int j=(queue->out); j<(queue->in-1); j++) {
      if (queue->processes[j]->prio < queue->processes[j+1]->prio) {
        temp_process = queue->processes[j];
        queue->processes[j] = queue->processes[j+1];
        queue->processes[j+1] = temp_process;
      }
    }
  }
}