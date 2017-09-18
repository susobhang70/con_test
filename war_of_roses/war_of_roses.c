#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#define maxsl 3

int n, capacity, linn = 0, sinn = 0, threadcount = 0;
pthread_mutex_t idlock;
sem_t fulllock, innlock, lock_first[2], lock_second[2];
int count_inside[2];

void lock_init()
{
    sem_init(&fulllock, 0, capacity);
    sem_init(&innlock, 0, 1);
    pthread_mutex_init(&idlock, NULL);
    int i;
    for(i = 0; i<2; i++)
    {
        sem_init(&lock_first[i], 0, 1);
        sem_init(&lock_second[i], 0, 1);
        count_inside[i] = 0;
    }
}

void getlock(int faction)
{
    sem_wait(&lock_first[faction]);
    sem_wait(&lock_second[faction]);
    count_inside[faction]++;
    if(count_inside[faction] == 1)
        sem_wait(&innlock);
    sem_post(&lock_second[faction]);
    sem_wait(&fulllock);
    sem_post(&lock_first[faction]);
}

void releaselock(int faction)
{
    sem_wait(&lock_second[faction]);
    if(count_inside[faction] == 1)
        sem_post(&innlock);
    count_inside[faction]--;
    sem_post(&lock_second[faction]);
    sem_post(&fulllock);
}

void *soldier(void *args)
{
    int faction = rand() % 2;
    pthread_mutex_lock(&idlock);
    int id = ++threadcount;
    pthread_mutex_unlock(&idlock);

    getlock(faction);
    //critical section
    if(faction == 0)
        printf("Soldier %d of Lannister inside inn\n", id);
    else
        printf("Soldier %d of Stark inside inn\n", id);
    sleep(1 + rand() % maxsl);

    if(faction == 0)
        printf("Soldier %d of Lannister moving out\n", id);
    else
        printf("Soldier %d of Stark moving out\n", id);

    releaselock(faction);

}


int main()
{
    printf("Enter the capacity of inn and total number of soldiers: ");
    scanf("%d %d", &capacity, &n);
    pthread_t p[n];
    int i, rc;
    lock_init();
    srand(time(NULL));

    printf("Initiate simulation\n");
    for(i = 0; i<n; i++)
    {
        rc = pthread_create(&p[i], NULL, soldier, NULL);
        assert (rc == 0);
    }

    for(i = 0; i<n; i++)
    {
        pthread_join(p[i], NULL);
    }
    printf("End simulation\n");
    return 0;
}
