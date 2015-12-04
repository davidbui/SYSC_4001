#include "helper.h"

// Initialize the queue with processes.
void init_queue(struct _queue *queue)
{
  queue->in = 0;
  queue->out = 0;
  queue->size = QUEUE_SZ;
}

// Adds a process to the queue of processes, and then re-orders them according to priority.
void append(struct _process_info *process, struct _queue *queue)
{
  queue->processes[queue->in] = *process;
  queue->in = (queue->in +1) % queue->size;
  reorder(queue);
}

// Consumes the highest priority process from the queue.
struct _process_info *take(struct _queue *queue)
{
  struct _process_info *p = (struct _process_info*)malloc(sizeof(struct _process_info));
  return p;
}
void reorder(struct _queue *queue)
{

}