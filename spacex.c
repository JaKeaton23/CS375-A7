// spacex.c
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cv   = PTHREAD_COND_INITIALIZER;
int n = 3;

void* counter(void* arg) {
    pthread_mutex_lock(&lock);
    while (n > 0) {
        printf("%d\n", n);
        n--;
        pthread_cond_signal(&cv);          // announcer may be waiting
        // give announcer a chance but keep holding protocol correct
        // unlock/lock to let announcer run if scheduled
        pthread_mutex_unlock(&lock);
        pthread_mutex_lock(&lock);
    }
    pthread_cond_broadcast(&cv);           // ensure announcer wakes at n==0
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* announcer(void* arg) {
    pthread_mutex_lock(&lock);
    while (n > 0) {
        pthread_cond_wait(&cv, &lock);     // wait until countdown reaches 0
    }
    printf("FALCON HEAVY TOUCH DOWN!\n");
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, counter, NULL);
    pthread_create(&t2, NULL, announcer, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
