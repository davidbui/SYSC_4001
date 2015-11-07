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

// Global variables.
int sem_id_s; // Provides mutual exclusion on buffer access.
int sem_id_n; // Synchronize producer and consumer on the number of consumable items.
int sem_id_e; // Synchronize producer and consumer on the number of empty spaces.

// Semaphore function prototypes.
int set_semvalue(int sem_id, int init_value);
void del_semvalue(int sem_id);
int semaphore_w(int sem_id);
int semaphore_s(int sem_id);

// Main program start.
int main(int argc, char *argv[])
{
  // Variables.
  int running = 1;
  void *shared_memory = (void *) 0;
  struct shared_used_st *shared_stuff;
  int shmid;

  srand((unsigned int)getpid());

  // Create a shared memory.
  shmid = shmget((key_t)1233, sizeof(struct shared_used_st), 0666 | IPC_CREAT);
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
  char inbuffer[TEXT_SZ];
  shared_stuff = (struct shared_used_st *)shared_memory;

  // Create semaphores onto the shared memory.
  sem_id_s = semget((key_t)1231, 1, 0666 | IPC_CREAT);
  sem_id_n = semget((key_t)1232, 1, 0666 | IPC_CREAT);
  sem_id_e = semget((key_t)1233, 1, 0666 | IPC_CREAT);

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


  // Implement producer logic here.
  while(running) {
    while(shared_stuff->written_by_you == 1) {
      sleep(1);
      printf("waiting for client\n");
    }
    printf("Enter some text: ");
    fgets(inbuffer, BUFSIZE, stdin);

    strncpy(shared_stuff->cbuffer[0].string, inbuffer, TEXT_SZ);
    shared_stuff->written_by_you = 1;

    if (strncmp(inbuffer, "end", 3) == 0) {
      running = 0;
    }
  }
  

  // Detach the shared memory from the current process.
  if (shmdt(shared_memory) == -1) {
    fprintf(stderr, "shmdt failed.\n");
    exit(EXIT_FAILURE);
  }

/*
 * Deletes the shared memory
  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed.\n");
    exit(EXIT_FAILURE);
  }
*/

  return 0;
}

// Semaphore function prototypes.
int set_semvalue(int sem_id, int init_value)
{
  union semun sem_union;

  sem_union.val = 1;
  if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
    return 0;
  }
  return 1;
}

void del_semvalue(int sem_id)
{
  union semun sem_union;

  sem_union.val = 1;
  if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
    fprintf(stderr, "Failed to delete semaphore.\n");
  }
}

int semaphore_w(int sem_id)
{
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = -1; 
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_w failed.\n");
    return 0;
  }
  return 1;
}

int semaphore_s(int sem_id)
{
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = 1; 
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_s failed.\n");
    return 0;
  }
  return 1;
}
