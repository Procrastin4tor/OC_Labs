#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "time.h"
#include "pthread.h"

#include "sys/ipc.h"
#include "sys/msg.h"

#include <iostream>

typedef struct {
	long mtype;
	int data[4];
} msgStruct;

void swap(int *a, int i, int j)
{
  int s = a[i];
  a[i] = a[j];
  a[j] = s;
}

void Print(int *a, int n)  // permutation output
{
  static int num = 1; // permutation number
  std::cout.width(3); // width of the output field of the permutation number
  std::cout << num++ << ": ";
  for (int i = 0; i < n; i++)
    std::cout << a[i] << " ";
  std::cout << std::endl;
}

bool NextSet(int *a, int n)
{
  int j = n - 2;
  while (j != -1 && a[j] >= a[j + 1]) j--;
    if (j == -1)
        return false; // no more permutations
  int k = n - 1;
  while (a[j] >= a[k]) k--;
    swap(a, j, k);
  int l = j + 1, r = n - 1; // sort the rest of the sequence
  while (l<r)
    swap(a, l++, r--);
  return true;
}

void* pthreadFunc(void* args)
{
	msgStruct pthreadMsg;
	int msgId = *((int *) args);

	ssize_t len = msgrcv(msgId, &pthreadMsg, sizeof(pthreadMsg), 0, 0);

	int arr[4];
	for(int i = 0; i<4; i++)
		arr[i] = pthreadMsg.data[i];
	
	while (NextSet(pthreadMsg.data, 4))
		for(int i = 0; i<4; i++)
			pthreadMsg.data[i] == arr[i];
        msgsnd(msgId, &pthreadMsg, sizeof(pthreadMsg), 0);

	return 0;
}

int main(void)
{
	int randNum[4];
	srand(time(NULL));
	for (int i = 0; i < 4; i++)
		randNum[i] = rand() % 9;

	//-------------Create msg--------------

	int msgId;
	if(msgId = msgget(IPC_PRIVATE, 0666 | IPC_CREAT) < 0)
	{
		perror("Error with msgget()!\n");
        return -1;
	}

	pthread_t thread;
	int res1 = pthread_create(&thread, NULL, pthreadFunc, &msgId);

	msgStruct parentMsg;

	parentMsg.mtype = 1;
	for (int i = 0; i < 4; i++)
		parentMsg.data[i] = randNum[i];

	msgsnd(msgId, &parentMsg, sizeof(parentMsg), 0);

	int msgStart = 0;
	int msgEnd = 24;

	while (msgStart != msgEnd)
	{
		ssize_t len = msgrcv(msgId, &parentMsg, sizeof(parentMsg), 0, 0);
		Print(parentMsg.data, 4);

		msgStart++;
	}


	msgctl(msgId, IPC_RMID, NULL);
	return 0;
}