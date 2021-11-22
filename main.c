#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#define PORT 6969
#define BUFFER_SIZE 16

int server_socket;
struct sockaddr_in address;
char buf[BUFFER_SIZE];
int bytes_read, total = 0;
char message_data[]={"Hello world"};
int client_socket;
struct sockaddr_in address;

void *client_runnable(const int *arg) {
    int sum = *(arg), value = *(arg + 1);

    printf("\nClient started\n %d %d", value, sum);

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    connect(client_socket, (struct sockaddr *) &address, sizeof(address));
    send(client_socket, message_data, strlen(message_data), 0);
    close(client_socket);
    pthread_exit(NULL);
}

void server_runnable() {
    printf("\nServer started\n");
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    int bind_result = bind(server_socket, (struct sockaddr *)&address, sizeof(address));
    printf("bind_result: %d", bind_result);
    while(1) {
        bytes_read = recvfrom(server_socket, buf, BUFFER_SIZE, 0, NULL, NULL);
        total = total + bytes_read;
        buf[bytes_read] = '\0';
        fprintf(stdout, "%s", buf);
        fflush(stdout);
        if (total > BUFFER_SIZE) {
            break;
        };
    }
    printf("\nTotal %d bytes received\n", total);
}

int main(int argc, char *argv[]) {
    printf("LAB 2:\nPopov N. 6133\n");
    printf("Enter 3 numbers (with enter key)");
    int a, b, c, sum;
    printf("Enter 1 number");
    scanf("%d", &a);
    printf("Enter 2 number");
    scanf("%d", &b);
    printf("Enter 3 number");
    scanf("%d", &c);
    printf("%d %d %d", a, b, c);
    sum = a + b + c;

    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, (void *(*)(void *)) client_runnable, (int[2]){a, sum});
    pthread_create(&thread2, NULL, (void *(*)(void *)) client_runnable, (int[2]){b, sum});
    pthread_create(&thread3, NULL, (void *(*)(void *)) client_runnable, (int[2]){c, sum});
    server_runnable();
    return 0;
}