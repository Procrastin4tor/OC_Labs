#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include <unistd.h>

void* printMessageFunction1()
{
   for (int i = 1; i < 11; i++)
      printf("Hello Thread(%i)\n", i), sleep(1);
   return NULL;
}

void* printMessageFunction2()
{
   for (int i = 1; i < 13; i++)
      printf("This is iteration %i\n", i), sleep(2);
   return NULL;
}

int main(void)
{
   pthread_t thread1, thread2;

   int res1 = pthread_create(&thread1, NULL, printMessageFunction1, NULL);
   int res2 = pthread_create(&thread2, NULL, printMessageFunction2, NULL);

   int iret1, iret2;

   pthread_join(thread1, (void**) &iret1);
   pthread_join(thread2, (void**) &iret2);

   return 0;
}