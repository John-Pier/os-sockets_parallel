#include <stdlib.h> /* Condition with mutex */
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

int data;
pthread_mutex_t mutex;
pthread_cond_t cond;

 void *Producer() {
    while(1) {
        fprintf(stderr, "\n1-st Wait...");
        pthread_mutex_lock(&mutex);
        fprintf(stderr, "\nPing: %d", data++);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

 void *Consumer() {
    while(1) {
        fprintf(stderr, "\n2-nd Wait...");
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        fprintf(stderr,"\nPong: %d", data--);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}
int main() {
    pthread_t thread1, thread2;
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    data = 0;
    pthread_create(&thread1, NULL, &Producer, NULL);
    pthread_create(&thread2, NULL, &Consumer, NULL);
    sleep(7);
    pthread_cond_destroy(&cond);
}
