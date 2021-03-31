#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "time.h"
#include "sys/wait.h"
#include "sys/stat.h"
#include "fcntl.h"

void print_array_int(int* arr, int size)
{
   	for (int i = 0; i < size; i++)
		printf("%d ", arr[i]);
	printf("\n");
}

int compare_int_value(const void* a, const void* b)
{
	return *((int*) b) - *((int*) a);
}

int main(int argv, char* argc[])
{
	if (argv <= 1)
	{
		printf("Error! Not enough params!\n");
		return -1;
	}

	int arrSize = atoi(argc[1]);
	int* arr = malloc(sizeof(int) * arrSize);

	srand(time(NULL));
	for (int i = 0; i < arrSize; i++)
	{
		arr[i] = rand() % 100;
		printf("%d ", arr[i]);
	}
	printf("\n");

    int fdPipe[2], fdFifo;
    size_t size;


	if (pipe(fdPipe) < 0)
	{
		printf("Error! Can't create pipe!\n");
		return -1;
	}

    const char* name = "arr.fifo";
	(void) umask(0);

	if (mknod(name, S_IFIFO | 0666, 0) < 0)
	{
		printf("Error! Can't create FIFO!\n");
		return -1;
	}

    pid_t childProcess = fork();


    if(childProcess < 0) // error create process
    {
        printf("Error! Can't fork child!\n");
		return -1;
    }
    else if(childProcess > 0) // process dad
    {
        close(fdPipe[1]);
        
        if((fdFifo = open(name, O_WRONLY))<0)
        {
            printf("Can`t open FFO for writing\n");
            exit(-1);
        }

        size = write(fdFifo, arr, sizeof(int) * arrSize);
        if(size < (sizeof(int) * arrSize))
        {
            printf("Can't write all array to FIFO!\n");
			return -1;
        }

        close(fdFifo);

        waitpid(childProcess, NULL, 0);

        size = read(fdPipe[0], arr, sizeof(int) * arrSize);
        if(size < 0)
        {
            printf("Can't read array!\n");
			return -1;
        }

        close(fdPipe[0]);

        print_array_int(arr, arrSize);

    }
    else // process child
    {
        close(fdPipe[0]);

        if((fdFifo = open(name, O_WRONLY))<0)
        {
            printf("Can`t open F for writing\n");
            exit(-1);
        }

        size = read(fdFifo, arr, sizeof(int) * arrSize);
        if(size < sizeof(int) * arrSize)
        {
            printf("Can`t read array");
            exit(-1);
        }

        close(fdFifo);

        qsort(arr, arrSize, sizeof(int), compare_int_value);

        size = write(fdPipe[1], arr, sizeof(int) * arrSize);
        if(size < sizeof(int) * arrSize)
        {
            printf("Can`t write arr in pipe");
            exit(-1);
        }
        close(fdPipe[1]);
    }

    char delete_fifo_file[124];
	sprintf(delete_fifo_file, "rm %s", name);
	system(delete_fifo_file);

    free(arr);
}
