/*
 * SHARED MEMORY
 * contains 100 buffers, each having 128 bytes (128 string of 128 elements)
 * contains 1 integer count
 * therefore, need a 100*128 byte + 4 byte buffer = 12,804 bytes
 * Producer
 * Repeatedly read text from a file and write the text into one of the buffers in shared memory until the end of file is reahed
 * use read() and write() - chapter 3
 *
 */

#include "helper.h"

/*  This function 'pops' a string from the circular buffer that is stored in shared memory.

  @param shared_stuff   This is a pointer to the shared memory used between the consumer and producer.
  @return char*         A pointer to the string that is 'removed' from the circular queue.
 */
char *take(struct shared_used_st *shared_stuff);
int bytes_read = 0;

// Main program start.
int main(int argc, char *argv[])
{
  // Variables.
  void *shared_memory = (void *) 0;
  struct shared_used_st *shared_stuff;
  int shmid;

  srand((unsigned int)getpid());

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
  shared_stuff->written_by_you = 0;
  shared_stuff->in = shared_stuff->out = 0;

  // Create/get semaphores on the shared memory.
  sem_id_s = semget((key_t)SEM_ID_S, 1, 0666 | IPC_CREAT);
  sem_id_n = semget((key_t)SEM_ID_N, 1, 0666 | IPC_CREAT);
  sem_id_e = semget((key_t)SEM_ID_E, 1, 0666 | IPC_CREAT);

/*
  // Initialize semaphores S, N, E.
  if (!set_semvalue(sem_id_s, 1)) {
    fprintf(stderr, "Failed to initialize semaphore.\n");
    exit(EXIT_FAILURE);
  }

  if (!set_semvalue(sem_id_n, 0)) {
    fprintf(stderr, "Failed to initialize semaphore.\n");
    exit(EXIT_FAILURE);
  }

  if (!set_semvalue(sem_id_e, CBUFFER_SZ)) {
    fprintf(stderr, "Failed to initialize semaphore.\n");
    exit(EXIT_FAILURE);
  }
*/

  int file_size = shared_stuff->file_size;
  char *s;

  // Implement consumer logic here.
  while(bytes_read < file_size) {
    fprintf(stderr, "bytes read = %d\n", bytes_read);

    fprintf(stderr, "wait(n)\n");
    semaphore_w(sem_id_n);  // Check if there is an item to consume from the buffer in shared memory.

    fprintf(stderr, "wait(s)\n");
    semaphore_w(sem_id_s);  // Check if buffer is being accessed. If not, lock it.

    fprintf(stderr, "take()\n");
    s = take(shared_stuff);


    fprintf(stderr, "signal(s)\n");
    semaphore_s(sem_id_s);  // Unlock buffer access.

    fprintf(stderr, "signal(e)\n");
    semaphore_s(sem_id_e);  // Add 1 free space to buffer.
    fprintf(stderr, "bytes read = %d\n", bytes_read);
    printf("Received message from producer:\n'%s'\n\n", s);
  }
  
  fprintf(stderr, "Finished consuming file from shared memory.\n");
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
  char *temp = shared_stuff->cbuffer[shared_stuff->out].string;
  bytes_read +=shared_stuff->cbuffer[shared_stuff->out].length;
  shared_stuff->out = (shared_stuff->out + 1) % CBUFFER_SZ;
  fprintf(stderr, "before shared_stuff->out = %d\n", shared_stuff->out);
  return temp;
}
