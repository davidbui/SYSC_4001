#ifndef HELPER_H
#define HELPER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/shm.h>
#include <sys/sem.h>

// Constants.
#define CBUFFER_SZ 100
#define BUFSIZE 128
#define TEXT_SZ 20
#define SEM_ID_S 1234
#define SEM_ID_N 1235
#define SEM_ID_E 1236

// Structs.
typedef struct _buffer {
  char string[BUFSIZE];
  int count;
} buffer;

struct shared_used_st {
  int written_by_you;
  buffer cbuffer[CBUFFER_SZ];
};

// Global variables.
int sem_id_s; // Provides mutual exclusion on buffer access.
int sem_id_n; // Synchronize producer and consumer on the number of consumable items.
int sem_id_e; // Synchronize producer and consumer on the number of empty spaces.

// Semaphore function prototypes.
int set_semvalue(int sem_id, int init_value);
void del_semvalue(int sem_id);
int semaphore_w(int sem_id);
int semaphore_s(int sem_id);

#endif
