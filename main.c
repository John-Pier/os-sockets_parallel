#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

#define PORT 6995

pthread_mutex_t mutex;
pthread_cond_t condSum, condMinus, condDivide, condPow;

int send_double(double num, int fd);

int receive_double(double *num, int fd);

void *divide_runnable(const double *arg) {
    double value = *(arg), divider = *(arg + 1);
    double result = value / divider;

    printf("\ndivide_runnable=%f\n", result);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("client_socket:socket");
        exit(1);
    }
    if (connect(client_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("client_socket:connect");
        exit(2);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&condDivide, &mutex);

    send_double(result, client_socket);

    pthread_mutex_unlock(&mutex);

    close(client_socket);
    pthread_exit(NULL);
}

void *sum_runnable(const double *arg) {
    struct sockaddr_in address;
    double result = 0;
    for (int i = 0; i < 3; i++) {
        result += arg[i];
    }

    printf("\nsum_runnable=%f\n", result);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0); // todo: use AF_UNIX
    if (client_socket < 0) {
        perror("client_socket:socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(client_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("client_socket:connect");
        exit(2);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&condSum, &mutex);

    send_double(result, client_socket);

    pthread_mutex_unlock(&mutex);

    close(client_socket);
    pthread_exit(NULL);
}

void *pow_runnable() {
    int i = 0;
    while (++i < 4) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condPow, &mutex);

        struct sockaddr_in address;
        int connectId;

        int client_socket = socket(AF_INET, SOCK_STREAM, 0); // todo: use AF_UNIX
        if (client_socket < 0) {
            perror("client_socket:socket");
            exit(1);
        }

        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        connectId = connect(client_socket, (struct sockaddr *) &address, sizeof(address));
        if (connectId < 0) {
            perror("client_socket:connect");
            exit(2);
        }
        double value;
        receive_double(&value, client_socket);
        double result = value * value;
        printf("\npow_runnable=%f", result);
        send_double(result, client_socket);
        close(client_socket);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *minus_runnable(const double average) {
    int i = 0;
    while (++i <= 3) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condMinus, &mutex);

        struct sockaddr_in address;
        int connectId;


        int client_socket = socket(AF_INET, SOCK_STREAM, 0); // todo: use AF_UNIX
        if (client_socket < 0) {
            perror("client_socket:socket");
            exit(1);
        }

        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        connectId = connect(client_socket, (struct sockaddr *) &address, sizeof(address));
        if (connectId < 0) {
            perror("client_socket:connect");
            exit(2);
        }

        double value = 0;
        receive_double(&value, client_socket);

        double part = value - average;
        double result = (part * part) / 2;
        printf("\nminus_runnable=%f", result);
        send_double(result, client_socket);

        close(client_socket);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *server_runnable(const double *arg) {
    double a = *(arg), b = *(arg + 1), c = *(arg + 2);
    double sum = 0;
    double average = 0;
    double *results_buffer = malloc(sizeof(double) * 3);

    pthread_t thread_sum, thread_pow, thread_minus, thread_divide;
    int thread1_status, thread2_status, thread3_status, thread4_status;
    /*
    pthread_create(&thread_sum, NULL, sum_runnable, (double[2]) {a, sum});
    pthread_create(&thread_pow, NULL, pow_runnable, (double[2]) {b, sum});
    pthread_create(&thread_minus, NULL, minus_runnable, (double[2]) {c, sum});
    pthread_create(&thread_divide, NULL, divide_runnable, (double[2]) {c, sum});
     */

    int server_socket;
    struct sockaddr_in address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("server_socket:socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("server_socket:bind");
        exit(2);
    }

    listen(server_socket, 3);
    printf("\nServer started\n");

    pthread_create(&thread_sum, NULL, sum_runnable, (double[3]) {a, b, c});
    int connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }
    pthread_cond_signal(&condSum);
    receive_double(&sum, connectId);
    printf("\nreceived sum: %f\n", sum);
    close(connectId);


    pthread_create(&thread_divide, NULL, divide_runnable, (double[3]) {sum, 3});
    connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }
    pthread_cond_signal(&condDivide);
    receive_double(&average, connectId);
    printf("\nreceived average: %f\n", average);
    close(connectId);

    pthread_create(&thread_minus, NULL, minus_runnable, &average);
    pthread_create(&thread_pow, NULL, pow_runnable, NULL);
    for (int i = 0; i < 3; i ++) {
        pthread_cond_signal(&condMinus);
        connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
        if (connectId < 0) {
            perror("accept");
            exit(3);
        }
        send_double(arg[i], connectId);
        receive_double(&results_buffer[i], connectId);
        printf("\nreceived: %f", results_buffer[i]);
        close(connectId);

        pthread_cond_signal(&condPow);
        connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
        if (connectId < 0) {
            perror("accept");
            exit(3);
        }
        send_double(results_buffer[i], connectId);
        receive_double(&results_buffer[i], connectId);
        printf("\nreceived: %f\n", results_buffer[i]);
        close(connectId);
    }

    //pthread_mutex_lock(&mutex);
    //pthread_cond_signal(&condSum);
    //pthread_mutex_unlock(&mutex);

    /*
    double sum = 0;
    receive_double(&sum, connectId);
    printf("\nreceived: %f\n", sum);
    results_buffer[i] = sum;
    */

    //TODO: wait finish of all connections
    double result = 0;
    for (int j = 0; j < 3; j++) {
        result += results_buffer[j];
    }
    printf("\nResult d: %f", result);

    pthread_join(thread_sum, (void **) &thread1_status);
    //pthread_join(thread_pow, (void **) &thread2_status);
    //pthread_join(thread_minus, (void **) &thread3_status);
    //pthread_join(thread_divide, (void **) &thread4_status);
    // pthread_join(server_thread, (void **) &thread3_status);
    exit(0);
}

int main(int argc, char *argv[]) {
    printf("LAB 2:\nPopov N. 6133\n");
    printf("Enter 3 numbers (with enter key)\n");
    double a, b, c;
    printf("Enter 1 number: ");
    scanf("%lf", &a);
    printf("Enter 2 number: ");
    scanf("%lf", &b);
    printf("Enter 3 number: ");
    scanf("%lf", &c);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condSum, NULL);
    pthread_cond_init(&condDivide, NULL);
    pthread_cond_init(&condPow, NULL);
    pthread_cond_init(&condMinus, NULL);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_runnable, (double[3]) {a, b, c});

    int status;
    pthread_join(server_thread, (void **) &status);
    pthread_cond_destroy(&condSum);
    pthread_cond_destroy(&condDivide);
    pthread_cond_destroy(&condPow);
    pthread_cond_destroy(&condMinus);
    return 0;
}

int send_double(double num, int fd) {
    char *data = (char *) &num;
    long left = sizeof(num);
    long rc;
    do {
        rc = send(fd, data, left, 0);
        if (rc < 0) {
            if (errno != EINTR) {
                return -1;
            }
        } else {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    return 0;
}

int receive_double(double *num, int fd) {
    double ret;
    char *data = (char *) &ret;
    long left = sizeof(ret);
    long rc;
    do {
        rc = recv(fd, data, left, 0);
        if (rc <= 0) {
            if (errno != EINTR) {
                return -1;
            }
        } else {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    *num = ret;
    return 0;
}
