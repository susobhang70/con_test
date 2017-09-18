#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <assert.h>

#define MAX 1000

int buffer[MAX], n;
int BUFSIZE, track, loops;
pthread_mutex_t idlock;

typedef struct _rwlock_t 
{
    sem_t lock; // binary semaphore (basic lock)
    sem_t writelock; // used to allow ONE writer or MANY readers
    sem_t empty;
    int readers; // count of readers reading in critical section
    int written; // if data has been written or ready to write.
    int read_by;
    pthread_mutex_t m;
    pthread_cond_t c;
} rwlock_t;

rwlock_t s[MAX];

void rwlock_init(rwlock_t *rw) 
{
    rw->readers = 0;
    rw->written = 0;
    rw->read_by = 0;
    pthread_mutex_init(&rw->m, NULL);
    pthread_cond_init(&rw->c, NULL);
    sem_init(&rw->lock, 0, 1);
    sem_init(&rw->writelock, 0, 1);
    sem_init(&rw->empty, 0, 1);
}

void rwlock_acquire_readlock(rwlock_t *rw, int count, int id) 
{
    pthread_mutex_lock(&rw->m);
    //printf("Condition lock by %d - value %d\n", id, count + 1);
    while(rw->written != count+1) // checks if the slot has been written afresh or not
    {    
        //printf("ID - %d Written - %d Loops - %d\n", id, rw->written, count + 1);
        pthread_cond_wait(&rw->c, &rw->m);
    }
    //printf("Condition unlock by %d\n", id);
    pthread_mutex_unlock(&rw->m);
    sem_wait(&rw->lock);
    //printf("Normal lock by %d\n", id);
    rw->readers++;
    if (rw->readers == 1)
        sem_wait(&rw->writelock); // first reader acquires writelock
    //printf("Normal unlock by %d\n", id);
    sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw) 
{
    sem_wait(&rw->lock);
    rw->readers--;
    sem_post(&rw->writelock);
    rw->read_by++;
    if (rw->read_by == n)
    {    
        sem_post(&rw->empty);
    } // last reader releases writelock and empties the slot in buffer
    sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw) 
{
    sem_wait(&rw->empty); // checks if the slot has been emptied by the readers after reading
    //printf("Empty lock acquired\n");
    sem_wait(&rw->writelock);
    //printf("Writelock acquired\n");
}

void rwlock_release_writelock(rwlock_t *rw) 
{
    pthread_mutex_lock(&rw->m);
    rw->written++;
    rw->read_by = 0;
    pthread_cond_broadcast(&rw->c);
    pthread_mutex_unlock(&rw->m);
    sem_post(&rw->writelock);
    //printf("Write finished\n");
}

void *reader(void *args)
{
    int count = 0, i, index = 0;
    pthread_mutex_lock(&idlock);
    int id = ++track;
    pthread_mutex_unlock(&idlock);
    //printf("My id is %d\n", id);
    for(i=0; i<loops; i++)
    {
        rwlock_acquire_readlock(&s[index], count, id);
        printf("Value read by %u at %d is %d\n", id, index, buffer[index]);
        rwlock_release_readlock(&s[index]);
        index++;
        if(index == BUFSIZE)
        {
            index = 0;
            count++;
        }
    }

}

void *writer(void *args)
{
    int i, index = 0;
    srand(time(NULL));
    for(i = 0; i<loops; i++)
    {
        rwlock_acquire_writelock(&s[index]);
        buffer[index] = rand() % BUFSIZE;
        printf("Value written at %d is %d\n", index, buffer[index]);
        rwlock_release_writelock(&s[index]);
        index = (index+1)%BUFSIZE;
    }
}

int main()
{
    int i, rc;
    printf("Enter size of buffer, number of readers and data entries: ");
    scanf("%d %d %d", &BUFSIZE, &n, &loops);
    track = -1;

    pthread_t p[n+1];

    for(i = 0; i<BUFSIZE; i++)
    {
        rwlock_init(&s[i]);
    }

    pthread_mutex_init(&idlock, NULL);

    printf("Thread creation begin\n");
    rc = pthread_create(&p[n], NULL, writer, NULL);
    assert (rc == 0);
    
    for(i = 0; i<n; i++)
    {
        rc = pthread_create(&p[i], NULL, reader, NULL);
        assert (rc == 0);
    }


    for(i = 0; i<n; i++)
    {
        pthread_join(p[i], NULL);
    }
    printf("Threads collected\n");

    return 0;
}