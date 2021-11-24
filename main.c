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

double* results_buffer;

pthread_mutex_t mutex;
pthread_cond_t cond;

int send_double(double num, int fd);
int receive_double(double *num, int fd);

void *client_runnable(const double *arg) {
    struct sockaddr_in address;
    double value = *(arg), sum = *(arg + 1);
    double part = sum - value;
    double result = (part * part) / 2;
    printf("\nClient result = %f\nvalue=%f\nsum=%f\n", result, value, sum);

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

   // pthread_mutex_lock(&mutex);
    //pthread_cond_wait(&cond, &mutex);

    send_double(result, client_socket);

   // pthread_mutex_unlock(&mutex);
    close(client_socket);
    pthread_exit(NULL);
}

void *server_runnable() {
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

    results_buffer = malloc(sizeof(double) * 3);

    int i = 0;
    while (i < 3) {

        int connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
        if (connectId < 0) {
            perror("accept");
            exit(3);
        }

        //pthread_mutex_lock(&mutex);

        double part_of_result = 0;
        receive_double(&part_of_result, connectId);
        printf("\nreceived: %f\n", part_of_result);
        results_buffer[i] = part_of_result;

        //pthread_cond_signal(&cond);
        //pthread_mutex_unlock(&mutex);

        close(connectId);
        i++;
    }

    //TODO: wait finish of all connections
    double result = 0;
    for (int j = 0; j < 3; j++) {
        result += results_buffer[j];
    }
    printf("\nResult d: %f", result);

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
    double sum = (a + b + c) / 3;
    printf("\nSum / 3 = %f", sum);

    pthread_t thread1, thread2, thread3, server_thread;
    int thread1_status, thread2_status, thread3_status;

    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&server_thread, NULL, server_runnable, NULL);

    pthread_create(&thread1, NULL, client_runnable, (double[2]) {a, sum});

    pthread_create(&thread2, NULL, client_runnable, (double[2]){b, sum});
    pthread_create(&thread3, NULL, client_runnable, (double[2]){c, sum});

    pthread_join(thread1, (void **) &thread1_status);
    pthread_join(thread2, (void **) &thread2_status);
    pthread_join(thread3, (void **) &thread3_status);
    pthread_join(server_thread, (void **) &thread3_status);

    pthread_cond_destroy(&cond);
    return 0;
}

int send_double(double num, int fd) {
    double conv = htole64(num);
    char *data = (char *) &conv;
    long left = sizeof(conv);
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
    *num = htole64(ret);
    return 0;
}
