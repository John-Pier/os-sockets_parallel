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

void *client_runnable() {
    printf("\nClient started\n");

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

int main() {
    printf("LAB 2:\nPopov N. 6133\n");
    pthread_t thread1, thread2, thread3;
    int result = pthread_create(&thread1, NULL, client_runnable, NULL);
    pthread_create(&thread2, NULL, client_runnable, NULL);
    pthread_create(&thread3, NULL, client_runnable, NULL);
    printf("\n%d\n", result);
    server_runnable();
    return 0;
}