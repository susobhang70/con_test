#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>

#include <time.h>

#define threshold 5
key_t key = IPC_PRIVATE;

void selectionsort(int *A, int l, int r)
{
    int i, j, min;
    for(i = l; i < r-1; i++)
    {
        min = i;
        for(j = l + 1; j < r; j++)
        {
            if(A[j] < A[min])
                min = j;
        }

        if(min != i)
        {
            int t = A[min];
            A[min] = A[i];
            A[i] = t;
        }

    }
}

void mergesort(int *A, int l, int r)
{
    if(r - l < threshold)
    {
        selectionsort(A, l, r);
        return;
    }

    int mid = (l + r)/2;
    pid_t pid, pid1;

    pid = fork();

    if(pid == -1)
    {
        printf("Couldn't create child process!");
        exit(EXIT_FAILURE);
    }

    else if(pid == 0)
    {
        mergesort(A, l, mid);
        exit(EXIT_SUCCESS);
    }

    pid1 = fork();

    if(pid1 == -1)
    {
        printf("Couldn't create child process!");
        exit(EXIT_FAILURE);
    }
    else if(pid1 == 0)
    {
        mergesort(A, mid, r);
        exit(EXIT_SUCCESS);
    }


    while (pid = waitpid(-1, NULL, 0)) 
    {
        if (errno == ECHILD) 
            break;
    }
    int B[r-l];
    int i = l, j = mid, k = 0;
    while(i != mid && j != r)
    {
        if(A[i] < A[j])
        {    
            B[k] = A[i];
            i++;
        }
        else
        {    
            B[k] = A[j];
            j++;
        }
        k++;
    }
    while(i != mid)
    {
        B[k] = A[i];
        k++;
        i++;
    }
    while(j != r)
    {
        B[k] = A[j];
        k++;
        j++;
    }
    for(i = l, k = 0; i<r; i++, k++)
        A[i] = B[k];

}

int main()
{
    int *sharedArray;
    int shmid, i, j, k, status, SHMSIZE;
    
    scanf("%d", &SHMSIZE);

    if((shmid=shmget(key, sizeof(int) *SHMSIZE, IPC_CREAT| 0666)) < 0)
    {
        perror("Shmget failed");
        exit(1);
    }
    
    if((sharedArray = shmat(shmid, 0, 0)) == (int *) -1)
    {
        perror("Shmat failed");
        exit(1);
    }

    for(i=0; i < SHMSIZE; i++)
        scanf("%d", &sharedArray[i]);

    // srand(time(NULL));
    // for(i = 0; i<SHMSIZE; i++)
    // {    
    //     sharedArray[i] = rand();
    //     printf("%d ", sharedArray[i]);
    // }

    
    mergesort(sharedArray, 0, SHMSIZE);

    for(i=0; i < SHMSIZE; i++)
        printf("%d ", sharedArray[i]);
    printf("\n");

    return 0;
}
