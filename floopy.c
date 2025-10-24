// floopy.c
#include <pthread.h>
#include <stdio.h>

typedef struct account_t {
    pthread_mutex_t lock;
    int balance;
    long uuid;
} account_t;

void transfer(account_t* donor, account_t* recipient, float amount) {
    // Total order by uuid prevents circular wait
    account_t* first  = (donor->uuid < recipient->uuid) ? donor : recipient;
    account_t* second = (donor->uuid < recipient->uuid) ? recipient : donor;

    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);

    if (donor->balance < amount) {
        printf("Insufficient funds.\n");
    } else {
        donor->balance    -= (int)amount;
        recipient->balance += (int)amount;
        printf("Transferred %.0f from account %ld to %ld\n",
               amount, donor->uuid, recipient->uuid);
    }

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

void* run_xfer(void* arg) {
    account_t** p = (account_t**)arg;
    transfer(p[0], p[1], 100);
    return NULL;
}

int main(void) {
    account_t a1 = {PTHREAD_MUTEX_INITIALIZER, 1000, 1};
    account_t a2 = {PTHREAD_MUTEX_INITIALIZER,  500, 2};

    pthread_t t1, t2;
    account_t* args1[2] = {&a1, &a2};
    account_t* args2[2] = {&a2, &a1};

    pthread_create(&t1, NULL, run_xfer, args1);
    pthread_create(&t2, NULL, run_xfer, args2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
