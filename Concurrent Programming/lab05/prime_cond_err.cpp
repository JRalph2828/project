#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREAD  10

int thread_ret[NUM_THREAD];

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int range_start;
int range_end;

bool is_done = false;


bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

void* ThreadFunc(void* arg) {
    long tid = (long)arg;


    int* find_me = (int*)malloc(sizeof(int) * 10);
    pthread_mutex_lock(&mutex);
    thread_ret[tid] = -1;
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);

    while (!is_done) {

        int start = range_start + ((range_end - range_start) / NUM_THREAD) * tid;
        int end = range_start + ((range_end - range_start) / NUM_THREAD) * (tid+1);
        if (tid == NUM_THREAD - 1) {
            end = range_end + 1;
        }

        long cnt_prime = 0;
        for (int i = start; i < end; i++) {
            if (IsPrime(i)) {
                cnt_prime++;
            }
        }


        pthread_mutex_lock(&mutex);
        thread_ret[tid] = cnt_prime;
        pthread_cond_wait(&cond, &mutex);

        pthread_mutex_unlock(&mutex);
    }
    free(find_me);

    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREAD];

    for (long i = 0; i < NUM_THREAD; i++) {

        if (pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0) {
            printf("pthread_create error!\n");
            return 0;
        }


        while (thread_ret[i] != -1) {
            pthread_yield();
        }
    }

    while (1) {
        scanf("%d", &range_start);
        if (range_start == -1) {
            is_done = true;

            pthread_mutex_lock(&mutex);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        }
        scanf("%d", &range_end);

        for (int i = 0; i < NUM_THREAD; i++) {
            thread_ret[i] = -1;
        }

        pthread_mutex_lock(&mutex);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);


        while (1) {
            bool all_thread_done = true;
            for (int i = 0; i < NUM_THREAD; i++) {
                if (thread_ret[i] < 0) {
                    all_thread_done = false;
                    break;
                }
            }
            if (all_thread_done) {
                break;
            }
            pthread_yield();
        }


        int number_of_prime = 0;
        for (int i = 0; i < NUM_THREAD; i++) {
            number_of_prime += thread_ret[i];
        }
        printf("number of prime: %d\n", number_of_prime);
    }


    for (int i = 0; i < NUM_THREAD; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}


