#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "time.h"
#include "stdlib.h"

#include "sys/shm.h"
#include "sys/types.h"
#include "sys/sem.h"
#include "sys/ipc.h"
#include "sys/wait.h"

#define SEMAPHORE_UNLOCK 1
#define SEMAPHORE_LOCK  -1

void* allocate_shared_memory(size_t memSize, int* memId)
{
	*memId = shmget(IPC_PRIVATE, memSize, 0600 | IPC_CREAT | IPC_EXCL);
	if (*memId <= 0)
	{
		perror("error with memId");
		return NULL;
	}

	void* mem = shmat(*memId, 0, 0);
	if (NULL == mem)
		perror("error with shmat");
	
	return mem;
}

void fullArr(int *array, int arrSize, int minValue, int maxValue)
{
    srand(time(NULL));
        for (int i = 0; i < arrSize; i++)
            array[i] = minValue+ rand()%maxValue;
}

void printArr(int* array, int size)
{
    for (size_t i = 0; i < size; i++)
        printf("%i ",array[i]);
}


//------------------------SemaphoreStart----------------------------------------


void semaphoreSetState(int semId, int num, int state)
{
	struct sembuf op;
	op.sem_op = state;
	op.sem_flg = 0;
	op.sem_num = num;
	semop(semId, &op, 1);
}

char semaphore_lock(int semId, int num, char* arrayCheck)
{
	if(arrayCheck[num])
        return 1;
	semaphoreSetState(semId, num, SEMAPHORE_LOCK);
	arrayCheck[num] = 1;
	return 0;
}

void semaphore_unlock(int semId, int num, char* arrayCheck)
{
	semaphoreSetState(semId, num, SEMAPHORE_UNLOCK);
	arrayCheck[num] = 0;
}

//------------------------SemaphoreEnd----------------------------------------



void childMainCode(int* array, char* arrayCheck, int size, int semId)
{
    int temp;

    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            semaphore_lock(semId, i, arrayCheck);
            semaphore_lock(semId, i+1, arrayCheck);
            if (array[j] > array[j + 1]) {
                temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
            semaphore_unlock(semId, i, arrayCheck);
            semaphore_unlock(semId, i+1, arrayCheck);
        }
    }
    
	exit(0);
}

void parentMainCode(int* array, char* arrayCheck, int size, int semId, pid_t child)
{
    int iteration = 0;
	while (!waitpid(child, NULL, WNOHANG))
	{
		printf("\nIteration %i\n\n", iteration);
		for (int i = 0; i < size; i++)
		{
			if (semaphore_lock(semId, i, arrayCheck))
				printf("Bloke\t");
			else
				printf("%d\t", array[i]);
			semaphore_unlock(semId, i, arrayCheck);
		}
		printf("\n");
		iteration++;
	}

	printf("Result: %i\n", iteration);
    printArr(array, size);
}

int main(int argv, char* argc[])
{
    if (argv <= 3)
	{
		printf("Error! Not enough arguments! Required: 3 (array_size, min, max)\n");
		return -1;
	}

    int arrSize = atoi(argc[1]);
    int minValue = atoi(argc[2]);
    int maxValue = atoi(argc[3]);

    int memId;
	int* array = allocate_shared_memory(sizeof(int) * arrSize, &memId);

    fullArr(array, arrSize, minValue, maxValue);



    int semId;
    if(semId = semget(IPC_PRIVATE, arrSize, 0600 | IPC_CREAT)<0){
        perror("Error with semget()!\n");
        return -1;
    }
    printf("Semaphore set id = %i\n", semId);

    int checkId;
	char* arrayCheck = allocate_shared_memory(sizeof(char) * arrSize, &checkId);

    for (size_t i = 0; i < arrSize; i++)
    {
        semaphoreSetState(semId, i, SEMAPHORE_UNLOCK);
    }



    pid_t childProcess = fork();

    if(childProcess < 0){
        perror("Error with fork() - process 1\n");
    }
    else if(childProcess > 0){
        parentMainCode(array, arrayCheck, arrSize, semId, childProcess);
    }
    else{
        childMainCode(array, arrayCheck, arrSize, semId);
    }
    


    char deleteCommand[124];
	sprintf(deleteCommand, "ipcrm -m %i", memId);
	system(deleteCommand);

    sprintf(deleteCommand, "ipcrm -m %i", checkId);
	system(deleteCommand);

	sprintf(deleteCommand, "ipcrm -s %i", semId);
	system(deleteCommand);
}