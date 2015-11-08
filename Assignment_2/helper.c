#include "helper.h"


// Semaphore function prototypes.
int set_semvalue(int sem_id, int init_value)
{
  union semun sem_union;

  sem_union.val = init_value;
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
  sem_b.sem_flg = 0;

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
  sem_b.sem_flg = 0;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_s failed.\n");
    return 0;
  }
  return 1;
}