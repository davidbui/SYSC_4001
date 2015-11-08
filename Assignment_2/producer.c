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

/*  This function adds a string to the circular buffer that is stored in shared memory.

  @param shared_stuff   This is a pointer to the shared memory used between the consumer and producer.
  @param string         This is a string that will be added to the circular queue.
 */
void append(struct shared_used_st *shared_stuff, char *string, int count);

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

  // Create semaphores onto the shared memory.
  sem_id_s = semget((key_t)SEM_ID_S, 1, 0666 | IPC_CREAT);
  sem_id_n = semget((key_t)SEM_ID_N, 1, 0666 | IPC_CREAT);
  sem_id_e = semget((key_t)SEM_ID_E, 1, 0666 | IPC_CREAT);

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

  // Read a file until it is empty. Each line is added to the circular buffer.
  char file_name[TEXT_SZ];
  printf("Enter the name of the file to append: ");
  fgets(file_name, sizeof(file_name), stdin);

  // Remove \n from input.
  char *pos;
  if ((pos = strchr(file_name, '\n')) != NULL) {
    *pos = '\0';
  }

  fprintf(stderr, "filename: '%s'\n", file_name);
  int input_fd;
  input_fd = open(file_name, O_RDONLY);

  if (input_fd == -1) {
    fprintf(stderr, "Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Opened '%s' succesfully\n", file_name);

  struct stat st;
  stat(file_name, &st);
  shared_stuff->file_size = st.st_size; // Let the consumer know how big the file is to know when to end.

  fprintf(stderr, "file size = %d\n", shared_stuff->file_size);

  char input_buffer[BUFSIZE];
  int len;
  int i;
  int total_s = 0;
  // Implement producer logic here to add string to the circular buffer..
  while((len = read(input_fd, &input_buffer, sizeof(input_buffer))) > 0) {
    fprintf(stderr, "len = %d\n", len);
    i = 0;

    do {
      fprintf(stderr, "wait(e)\n");
      semaphore_w(sem_id_e);                      // Check if there is space left to append on the circular buffer.
      
      fprintf(stderr, "wait(s)\n");
      semaphore_w(sem_id_s);                      // Lock the circular buffer.
      
      fprintf(stderr, "append()\n");
      if (len > 128) {
        append(shared_stuff, input_buffer + (i*128), BUFSIZE);    // Add string to the circular buffer and the length of the string.
        fprintf(stderr, "Wrote %d bytes to the shared memory.\n", BUFSIZE);
        total_s+=128;
      } else {
        append(shared_stuff, input_buffer + (i*128), len); 
        fprintf(stderr, "Wrote %d bytes to the shared memory.\n", len);
        total_s+=len;
      }

      i++;
      len = len-128;

      fprintf(stderr, "signal(s)\n");
      semaphore_s(sem_id_s);                      // Unlock the circular buffer.
      
      fprintf(stderr, "signal(n)\n");
      semaphore_s(sem_id_n);                      // Let the consumer know there is an item avaiable.
    } while(len > 0);
  }

  fprintf(stderr, "Finished producing file to shared memory.\n");

  // Detach the shared memory from the current process.
  if (shmdt(shared_memory) == -1) {
    fprintf(stderr, "shmdt failed.\n");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "totals = %d\n", total_s);
;/*
 * Deletes the shared memory
  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed.\n");
    exit(EXIT_FAILURE);
  }
*/

  return 0;
}

void append(struct shared_used_st *shared_stuff, char *string, int length)
{
  fprintf(stderr, "before shared_stuff->in = %d\n", shared_stuff->in);
  memcpy(shared_stuff->cbuffer[shared_stuff->in].string, string, BUFSIZE);  // Copy the string to the circular buffer.
  shared_stuff->cbuffer[shared_stuff->in].length = length;                  // Add the length of the string to the buffer.
  shared_stuff->in = (shared_stuff->in +1) % CBUFFER_SZ;                    // Update the next append index.
  fprintf(stderr, "after shared_stuff->in = %d\n", shared_stuff->in);
}
