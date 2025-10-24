// hello_world.c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t lock;
pthread_cond_t  cv;
int hello = 0;

void* print_hello(void* arg) {
    pthread_mutex_lock(&lock);
    hello += 1;
    printf("First line (hello=%d)\n", hello);
    pthread_cond_signal(&cv);          // wake the waiter
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void) {
    pthread_t t;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv, NULL);

    pthread_create(&t, NULL, print_hello, NULL);

    pthread_mutex_lock(&lock);
    while (hello == 0) {               // Mesa semantics: use a while loop
        pthread_cond_wait(&cv, &lock);
    }
    printf("Second line (hello=%d)\n", hello);
    pthread_mutex_unlock(&lock);

    pthread_join(t, NULL);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cv);
    return 0;
}
