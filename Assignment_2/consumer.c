#include "helper.h"

// Functions
/*  This function 'pops' a string from the circular buffer that is stored in shared memory.

  @param shared_stuff   This is a pointer to the shared memory used between the consumer and producer.
  @return char*         A pointer to the string that is 'removed' from the circular queue.
 */
char *take(struct shared_used_st *shared_stuff, int output_fd);

// Global variables.
int total_bytes_read = 0;
int total_bytes_wrote = 0;

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


  // Open the file to be produced/consumed.
  char file_name[TEXT_SZ];
  printf("Enter the name of the new file created from consuming: ");
  fgets(file_name, sizeof(file_name), stdin);

  // Remove '\n' from input.
  char *pos;
  if ((pos = strchr(file_name, '\n')) != NULL) {
    *pos = '\0';
  }

  int output_fd = open(file_name, O_WRONLY|O_CREAT);
  if (output_fd == -1) {
    fprintf(stderr, "Error while opening the file.\n"); 
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Opened '%s' succesfully\n", file_name);

  // Implement consumer logic here.
  while(total_bytes_read < file_size) {
    fprintf(stderr, "p_wait(n)\n");
    p_wait(sem_id_n);  // Check if there is an item to consume from the buffer in shared memory.

    //fprintf(stderr, "p_wait(s)\n");
    //p_wait(sem_id_s);  // Check if buffer is being accessed. If not, lock it.

    fprintf(stderr, "take()\n");
    out_string = take(shared_stuff, output_fd);

    //fprintf(stderr, "p_signal(s)\n");
    //p_signal(sem_id_s);  // Unlock buffer access.

    fprintf(stderr, "p_signal(e)\n");
    p_signal(sem_id_e);  // Add 1 free space to buffer.

    //printf("Received message from producer:\n'%s'\n\n", out_string);
  }
  
  // Get the size of the file new file.
  struct stat st;
  stat(file_name, &st);
  int new_file_size = st.st_size;

  fprintf(stderr, "Finished consuming file from shared memory.\n");
  close(output_fd);
  //fprintf(stderr, "Total number of bytes read = %d\n", total_bytes_read);
  //fprintf(stderr, "Total number of bytes of new file created (%s) = %d\n", file_name, new_file_size);

  if (shared_stuff->file_size == new_file_size) {
    fprintf(stderr, "The producer file and the new file created by the consumer are the exact same!\n");
  } else {
    fprintf(stderr, "The producer file and the new file created by the consumer are not the same!\n\n");
  }

  fprintf(stderr, "-> Producer file size    : %d bytes\n", shared_stuff->file_size);
  fprintf(stderr, "-> New consumer file size: %d bytes\n\n", file_size);

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

char *take(struct shared_used_st *shared_stuff, int output_fd)
{
  fprintf(stderr, "Current out index = %d\n", shared_stuff->out);
  char *temp = shared_stuff->cbuffer[shared_stuff->out].string;                                   // Grab current string at out index.
  int bytes_wrote = write(output_fd, temp, shared_stuff->cbuffer[shared_stuff->out].length);      // Write to new file.
  total_bytes_wrote += bytes_wrote;                                                               // Update number total number of bytes wrote. 
  fprintf(stderr, "Wrote to new file: %d bytes\n", bytes_wrote);

  // A check as required in the assignment.
  if (bytes_wrote != shared_stuff->cbuffer[shared_stuff->out].length) {
    fprintf(stderr, "Error: The number of bytes consumed and written are not the same!\n");
  }

  total_bytes_read +=shared_stuff->cbuffer[shared_stuff->out].length;                             // Update number total number of bytes read.
  shared_stuff->out = (shared_stuff->out + 1) % CBUFFER_SZ;                                       // Increment the out index.
  return temp;
}
