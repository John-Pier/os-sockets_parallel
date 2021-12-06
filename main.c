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

#define PORT 6927

pthread_mutex_t mutex;
pthread_mutex_t main_mutex;
pthread_cond_t condSum, condMinus, condDivide, condPow;
pthread_t thread_sum, thread_pow, thread_minus, thread_divide;

int send_double(double num, int fd);

int receive_double(double *num, int fd);

void *divide_runnable() {
    printf("\ndivide_runnable start");
    int i = 0;
    while (++i < 3) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condDivide, &mutex);

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

        double value = 0;
        double divider = 0;
        receive_double(&value, client_socket);
        receive_double(&divider, client_socket);

        double result = value / divider;
        printf("\nOperation: %f/%f=%f\n", value, divider, result);

        send_double(result, client_socket);

        close(client_socket);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *sum_runnable() {
    printf("\nsum_runnable start");
    int i = 0;
    while (++i < 3) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condSum, &mutex);

        struct sockaddr_in address;
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

        double result = 0;
        double value;
        printf("\nsum[");
        for (int j = 0; j < 3; j++) {
            receive_double(&value, client_socket);
            printf("%f,", value);
            result += value;
        }
        printf("]\n");

        send_double(result, client_socket);
        printf("\nOperation: sum[]=%f\n", result);

        close(client_socket);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *pow_runnable() {
    printf("\npow_runnable start");
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
        printf("\nOperation: %f^2=%f\n", value, result);
        send_double(result, client_socket);
        close(client_socket);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *minus_runnable() {
    printf("\nminus_runnable start");
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
        double average = 0;
        receive_double(&value, client_socket);
        receive_double(&average, client_socket);

        double result = value - average;
        printf("\nOperation: %f-%f=%f\n", value, average, result);
        send_double(result, client_socket);

        close(client_socket);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *server_runnable(const double *arg) {
    //pthread_mutex_lock(&main_mutex);
    //double a = *(arg), b = *(arg + 1), c = *(arg + 2);
    double sum = 0;
    double average = 0;
    double *results_buffer = malloc(sizeof(double) * 3);
    double result;

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

    listen(server_socket, 10);
    printf("\nServer started\n");

    pthread_cond_signal(&condSum);
    int connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }
    for (int i = 0; i < 3; i++) {
        send_double(arg[i], connectId);
    }
    receive_double(&sum, connectId);
    printf("\nSum: received: %f\n", sum);
    close(connectId);

    pthread_cond_signal(&condDivide);
    connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }
    send_double(sum, connectId);
    send_double(3, connectId);
    receive_double(&average, connectId);
    printf("\nDiv: received: %f\n", average);
    close(connectId);

    for (int i = 0; i < 3; i ++) {
        pthread_cond_signal(&condMinus);
        connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
        if (connectId < 0) {
            perror("accept");
            exit(3);
        }
        send_double(arg[i], connectId);
        send_double(average, connectId);
        receive_double(&results_buffer[i], connectId);
        printf("\nMinus: received: %f\n", results_buffer[i]);
        close(connectId);

        pthread_cond_signal(&condPow);
        connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
        if (connectId < 0) {
            perror("accept");
            exit(3);
        }
        send_double(results_buffer[i], connectId);
        receive_double(&results_buffer[i], connectId);
        printf("\nPow: received: %f\n", results_buffer[i]);
        close(connectId);
    }

    pthread_cond_signal(&condSum);
    connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }
    for (int i = 0; i < 3; i++) {
        send_double(results_buffer[i], connectId);
    }
    receive_double(&sum, connectId);
    printf("\nSum: received: %f\n", sum);
    close(connectId);

    pthread_cond_signal(&condDivide);
    connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }
    send_double(sum, connectId);
    send_double(2, connectId);
    receive_double(&result, connectId);
    printf("\nDiv: received: %f\n", result);
    close(connectId);

    printf("\n-----\nResult: %f", result);
    pthread_exit(NULL);
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
    pthread_mutex_init(&main_mutex, NULL);

    pthread_cond_init(&condSum, NULL);
    pthread_cond_init(&condDivide, NULL);
    pthread_cond_init(&condPow, NULL);
    pthread_cond_init(&condMinus, NULL);

    pthread_create(&thread_sum, NULL, sum_runnable, NULL);
    pthread_create(&thread_divide, NULL, divide_runnable, NULL);
    pthread_create(&thread_minus, NULL, minus_runnable, NULL);
    pthread_create(&thread_pow, NULL, pow_runnable, NULL);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_runnable, (double[3]) {a, b, c});

    sleep(1);
    int status;
    int thread1_status, thread2_status, thread3_status, thread4_status;
    pthread_join(server_thread, (void **) &status);
    pthread_detach(thread_sum);
    pthread_detach(thread_pow);
    pthread_detach(thread_minus);
    pthread_detach(thread_divide);
    //pthread_join(thread_sum, (void **) &thread1_status);
    //pthread_join(thread_pow, (void **) &thread2_status);
    //pthread_join(thread_minus, (void **) &thread3_status);
    //pthread_join(thread_divide, (void **) &thread4_status);
    pthread_cond_destroy(&condDivide);
    pthread_cond_destroy(&condSum);
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
