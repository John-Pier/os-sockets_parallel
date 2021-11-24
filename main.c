#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>

#define PORT 6969

long send_all(int s, char *buf, int len, int flags) {
    long total = 0;
    long n;
    while (total < len) {
        n = send(s, buf + total, len - total, flags);
        if (n == -1) { break; }
        total += n;
    }
    return (n == -1 ? -1 : total);
}

long recive_all(int s, char *buf, int len, int flags) {
    long total = 0;
    long n;
    while (total < len) {
        n = recv(s, buf + total, len - total, flags);
        if (n == -1) { break; }
        total += n;
    }
    return (n == -1 ? -1 : total);
}

void init_address(struct sockaddr_un *addr, const char *path, int I) {
    struct sockaddr_un tmp = {.sun_family = AF_UNIX};
    // fill sun_path
    memset(tmp.sun_path, I, 108);
    // copy path
    snprintf(tmp.sun_path, 108, "%s", path);
    *addr = tmp;
}


void *client_runnable(const double *arg) {
    struct sockaddr_in address;
    char message[] = "Hello there!\n";
    char buf[sizeof(message)];
    //init_address(&address, "/tmp/test", 'x');
    //address.sun_family = AF_UNIX;

    double sum = *(arg), value = *(arg + 1);
    double part = sum - value;
    double result = (part * part) / 2;
    printf("\nClient result = %f\n", result);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0); // todo: use AF_UNIX
    if (client_socket < 0) {
        perror("client_socket:socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);


    if (connect(client_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("\n Error : Connect Failed \n");
        perror("client_socket:connect");
        exit(2);
    }
    send(client_socket, message, sizeof(message), 0);
    close(client_socket);
    pthread_exit(NULL);
}

void server_runnable() {
    long bytes_read, current = 0;
    //int8_t BUFFER_SIZE = sizeof(double);
    //double part_of_result;
    //double* results_buffer = malloc(sizeof(double) * 3);

    int server_socket;
    struct sockaddr_in address;
    //init_address(&address, "/tmp/test", 'x');
    //address.sun_family = AF_UNIX;
    //address.sun_path = '\0';

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

    int connectId = accept(server_socket, (struct sockaddr *) NULL, NULL);
    if (connectId < 0) {
        perror("accept");
        exit(3);
    }

    char buf[1024];

    //char *buf = malloc(sizeof(double)+1);
    while (1) {
        bytes_read = recv(connectId, buf, 1024, 0);
        if (bytes_read <= 0) break;
    }
    printf("buf %s", buf);

    close(connectId);
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
//    printf("%f %f %f", a, b, c);
    double sum = (a + b + c) / 3;
    //printf("\n%f", sum);

    //server_runnable();
    pthread_t thread1, thread2, thread3;
    int thread1_status, thread2_status, thread3_status;
    pthread_create(&thread2, NULL, server_runnable, NULL);

    pthread_create(&thread1, NULL, client_runnable, (double[2]) {a, sum});

    //pthread_create(&thread2, NULL, client_runnable, (double[2]){b, sum});
    //pthread_create(&thread3, NULL, client_runnable, (double[2]){c, sum});
    pthread_join(thread1, (void **) &thread1_status);
    //pthread_join(thread2, (void **) &thread2_status);
    // pthread_join(thread3, (void **) &thread3_status);
    return 0;
}