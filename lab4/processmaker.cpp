#include "stdio.h"
#include "sys/shm.h"
#include "stdlib.h"
#include "sys/ipc.h"
#include "unistd.h"
#include "time.h"
#include <sys/types.h>
#include <sys/wait.h>

void* allocate_shared_memory(size_t memSize, int& memId)
{
   memId = shmget(IPC_PRIVATE, memSize, 0600 | IPC_CREAT | IPC_EXCL);
   if (memId <= 0)
   {
      perror("error with memId");
      return NULL;
   }

   void* mem = shmat(memId, 0, 0);
   if (NULL == mem)
      perror("error with shmat");
   
   return mem;
}

int compare_int_value(const void* a, const void* b)
{
   return *((int*) a) - *((int*) b);
}

void printArray(int* sharedMem)
{
    for(int i = 0; i < 20; i++)
        printf("%i ", *(sharedMem +i));
        printf("\n");
}

void childMainCode(int* sharedMem)
{
    qsort(sharedMem, 20, 4, compare_int_value);
    printArray(sharedMem);
}

int main(void)
{
    int memId;
    int* sharedMem = (int*) allocate_shared_memory(128, memId);
    printf("mem_id = %d\n", memId);

    srand(time(NULL));
    for (int i = 0; i < 20; i++)
        *(sharedMem + i) = rand() % 50;
    printArray(sharedMem);

    pid_t childId = fork();

    if(childId < 0)
        perror("error with fork()\n");
    else if(childId > 0)
        waitpid(childId, NULL, 0);
    else
        childMainCode(sharedMem);

    char callbuf[128];
    sprintf(callbuf, "ipcrm -m %i", memId);
    system(callbuf);

    return 0;
}