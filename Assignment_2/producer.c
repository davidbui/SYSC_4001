#include "helper.h"

// Functions
/*  This function adds a string to the circular buffer that is stored in shared memory.

  @param shared_stuff   This is a pointer to the shared memory used between the consumer and producer.
  @param string         This is a string that will be added to the circular queue.
 */
void append(struct shared_used_st *shared_stuff, char *string, int count);


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
  shared_stuff->in = shared_stuff->out = 0;

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

  // Open the file to be produced/consumed.
  char file_name[TEXT_SZ];
  printf("Enter the name of the file to append: ");
  fgets(file_name, sizeof(file_name), stdin);

  // Remove '\n' from input.
  char *pos;
  if ((pos = strchr(file_name, '\n')) != NULL) {
    *pos = '\0';
  }

  int input_fd = open(file_name, O_RDONLY);
  if (input_fd == -1) {
    fprintf(stderr, "Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Opened '%s' succesfully\n", file_name);

  // Get the size of the file.
  struct stat st;
  stat(file_name, &st);
  shared_stuff->file_size = st.st_size; // Let the consumer know how big the file is to know when to end.

  fprintf(stderr, "File size of %s = %d\n", file_name, shared_stuff->file_size);

  char input_buffer[BUFSIZE];
  int i, in_length, total_write_size = 0;

  // Implement producer logic here to add string to the circular buffer..
  while((in_length = read(input_fd, &input_buffer, sizeof(input_buffer))) > 0) {
    fprintf(stderr, "in_length = %d\n", in_length);
    i = 0;

    // Includes logic that splits up the 1024 byte sized input into pieces of 128.
    do {
      fprintf(stderr, "p_wait(e)\n");
      p_wait(sem_id_e);                   // Check if there is space left to append on the circular buffer.
      fprintf(stderr, "p_wait(s)\n");
      p_wait(sem_id_s);                   // Lock the circular buffer.
      fprintf(stderr, "append()\n");

      if (in_length > 128) {
        append(shared_stuff, input_buffer + (i*128), BUFSIZE);    // Add string to the circular buffer and the length of the string.
        fprintf(stderr, "Bytes written to circular buffer in shared memory: %d\n", BUFSIZE);
        total_write_size+=128;
      } else {
        append(shared_stuff, input_buffer + (i*128), in_length); 
        fprintf(stderr, "Bytes written to circular buffer in shared memory: %d\n", in_length);
        total_write_size+=in_length;
      }

      i++;
      in_length = in_length-128;

      fprintf(stderr, "p_signal(s)\n");
      p_signal(sem_id_s);                // Unlock the circular buffer.
      fprintf(stderr, "p_signal(n)\n\n");
      p_signal(sem_id_n);                // Let the consumer know there is an item avaiable.
    } while(in_length > 0);
  }

  fprintf(stderr, "Finished producing file to shared memory.\n");

  // Detach the shared memory from the current process.
  if (shmdt(shared_memory) == -1) {
    fprintf(stderr, "shmdt failed.\n");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "total_write_size = %d\n", total_write_size);

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
