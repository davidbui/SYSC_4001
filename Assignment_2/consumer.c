#include "helper.h"

// Functions
/*  This function 'pops' a string from the circular buffer that is stored in shared memory.

  @param shared_stuff   This is a pointer to the shared memory used between the consumer and producer.
  @return char*         A pointer to the string that is 'removed' from the circular queue.
 */
char *take(struct shared_used_st *shared_stuff);

// Global variables.
int total_bytes_read = 0;


// Main program start.
int main(int argc, char *argv[])
{
  // Variables.
  void *shared_memory = (void *) 0;
  struct shared_used_st *shared_stuff;
  int shmid;

  // Create a shared memory.
  shmid = shmget((key_t)SHM_ID, sizeof(struct shared_used_st), 0666 | IPC_CREAT);
  if (shmid == -1) {
    fprintf(stderr, "shmget failed.\n");
    exit(EXIT_FAILURE);
  }

  // Attach the shared memory to this process.
  shared_memory = shmat(shmid, (void *)0, 0);
  if (shared_memory == (void *)-1) {
    fprintf(stderr, "shmat failed.\n");
    exit(EXIT_FAILURE);
  }

  printf("Memory attached at %X\n", (int)shared_memory);
  shared_stuff = (struct shared_used_st *)shared_memory;

  // Create/get semaphores on the shared memory.
  sem_id_s = semget((key_t)SEM_ID_S, 1, 0666 | IPC_CREAT);
  sem_id_n = semget((key_t)SEM_ID_N, 1, 0666 | IPC_CREAT);
  sem_id_e = semget((key_t)SEM_ID_E, 1, 0666 | IPC_CREAT);

  int file_size = shared_stuff->file_size;
  char *out_string;

  // Implement consumer logic here.
  while(total_bytes_read < file_size) {
    fprintf(stderr, "p_wait(n)\n");
    p_wait(sem_id_n);  // Check if there is an item to consume from the buffer in shared memory.

    fprintf(stderr, "p_wait(s)\n");
    p_wait(sem_id_s);  // Check if buffer is being accessed. If not, lock it.

    fprintf(stderr, "take()\n");
    out_string = take(shared_stuff);

    fprintf(stderr, "p_signal(s)\n");
    p_signal(sem_id_s);  // Unlock buffer access.

    fprintf(stderr, "p_signal(e)\n");
    p_signal(sem_id_e);  // Add 1 free space to buffer.

    printf("Received message from producer:\n'%s'\n\n", out_string);
  }
  
  fprintf(stderr, "Finished consuming file from shared memory.\n");
  fprintf(stderr, "Total number of bytes read = %d\n", total_bytes_read);

  // Detach the shared memory from the current process.
  if (shmdt(shared_memory) == -1) {
    fprintf(stderr, "shmdt failed.\n");
    exit(EXIT_FAILURE);
  }

  //Deletes the shared memory
  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed.\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}

char *take(struct shared_used_st *shared_stuff)
{
  fprintf(stderr, "before shared_stuff->out = %d\n", shared_stuff->out);
  char *temp = shared_stuff->cbuffer[shared_stuff->out].string;   // Grab current string at out index.
  total_bytes_read +=shared_stuff->cbuffer[shared_stuff->out].length;   // Update number total number of bytes read.
  shared_stuff->out = (shared_stuff->out + 1) % CBUFFER_SZ;       // Update the next append index.
  fprintf(stderr, "before shared_stuff->out = %d\n", shared_stuff->out);
  return temp;
}
