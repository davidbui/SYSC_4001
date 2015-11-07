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

// Structs.
typedef struct _buffer {
  char string[BUFSIZE];
  int count;
} buffer;

struct shared_used_st {
  int written_by_you;
  buffer cbuffer[CBUFFER_SZ];
};


#endif
