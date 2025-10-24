// priority_transfer.c
#include <pthread.h>
#include <stdio.h>

typedef struct account_t {
    pthread_mutex_t lock;
    int balance;
    long uuid;
    int priority;           
} account_t;

// naive donation: raise the priority field on the lock holder we're waiting on
static void donate_if_needed(account_t* a, int thread_priority) {
    if (a->priority < thread_priority) a->priority = thread_priority;
}

void transfer(account_t* donor, account_t* recipient, int amount, int thread_priority) {
    account_t* first  = (donor->uuid < recipient->uuid) ? donor : recipient;
    account_t* second = (donor->uuid < recipient->uuid) ? recipient : donor;

    donate_if_needed(first, thread_priority);
    pthread_mutex_lock(&first->lock);
    first->priority = thread_priority;

    donate_if_needed(second, thread_priority);
    pthread_mutex_lock(&second->lock);
    second->priority = thread_priority;

    if (donor->balance >= amount) {
        donor->balance    -= amount;
        recipient->balance += amount;
        printf("[P%d] transfer %d from %ld to %ld\n",
               thread_priority, amount, donor->uuid, recipient->uuid);
    } else {
        printf("[P%d] insufficient funds\n", thread_priority);
    }

    second->priority = 0;
    pthread_mutex_unlock(&second->lock);
    first->priority  = 0;
    pthread_mutex_unlock(&first->lock);
}

typedef struct {
    account_t* d; account_t* r; int amt; int pr;
} args_t;

void* transfer_thread(void* arg) {
    args_t* a = (args_t*)arg;
    transfer(a->d, a->r, a->amt, a->pr);
    return NULL;
}

int main(void) {
    account_t a1 = {PTHREAD_MUTEX_INITIALIZER, 1000, 1, 0};
    account_t a2 = {PTHREAD_MUTEX_INITIALIZER,  500, 2, 0};

    pthread_t hi, lo;
    args_t hi_args = {&a1, &a2, 200, 20}; // high priority
    args_t lo_args = {&a2, &a1, 100,  5}; // low priority (could hold a lock)

    pthread_create(&lo, NULL, transfer_thread, &lo_args);
    pthread_create(&hi, NULL, transfer_thread, &hi_args);

    pthread_join(hi, NULL);
    pthread_join(lo, NULL);
    return 0;
}
