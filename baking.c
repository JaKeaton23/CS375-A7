// baking.c
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

int numBatterInBowl = 0;
int numEggInBowl    = 0;
bool readyToEat     = false;

pthread_mutex_t lock;
pthread_cond_t needIngredients, readyToBake, startEating;

void addBatter(void) { numBatterInBowl += 1; }
void addEgg(void)    { numEggInBowl    += 1; }
void heatBowl(void)  { readyToEat = true; numBatterInBowl = 0; numEggInBowl = 0; }
void eatCake(void)   { readyToEat = false; }

void* batterAdder(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (numBatterInBowl >= 1 || readyToEat)             // only 1 batter
            pthread_cond_wait(&needIngredients, &lock);
        addBatter();
        pthread_cond_signal(&readyToBake);                      // maybe enough now
        pthread_mutex_unlock(&lock);
        usleep(10000);
    }
    return NULL;
}

void* eggBreaker(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (numEggInBowl >= 2 || readyToEat)                 // max 2 eggs
            pthread_cond_wait(&needIngredients, &lock);
        addEgg();
        pthread_cond_signal(&readyToBake);
        pthread_mutex_unlock(&lock);
        usleep(10000);
    }
    return NULL;
}

void* bowlHeater(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (!(numBatterInBowl >= 1 && numEggInBowl >= 2) || readyToEat)
            pthread_cond_wait(&readyToBake, &lock);
        heatBowl();
        pthread_cond_signal(&startEating);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* cakeEater(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (!readyToEat)
            pthread_cond_wait(&startEating, &lock);
        eatCake();
        printf("Cake baked & eaten!\n");
        pthread_cond_broadcast(&needIngredients); // let producers add again
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void) {
    pthread_t t_batter, t_egg1, t_egg2, t_heater, t_eater;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&needIngredients, NULL);
    pthread_cond_init(&readyToBake, NULL);
    pthread_cond_init(&startEating, NULL);

    pthread_create(&t_batter, NULL, batterAdder, NULL);
    pthread_create(&t_egg1,   NULL, eggBreaker, NULL);
    pthread_create(&t_egg2,   NULL, eggBreaker, NULL);
    pthread_create(&t_heater, NULL, bowlHeater, NULL);
    pthread_create(&t_eater,  NULL, cakeEater, NULL);

    while (1) sleep(1); // run forever for demo
    return 0;
}
