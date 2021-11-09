//
// Created by nik2100 on 10.11.2021.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>

#define PORT 6969
#define BUFFER_SIZE 16

int server_socket;
struct sockaddr_in address;
char buf[BUFFER_SIZE];
int bytes_read, total = 0;

int main() {
    printf("Hello, World!");
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    int bind_result = bind(server_socket, (struct sockaddr *)&address, sizeof(address));
    printf("bind_result: %s", bind_result);
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
    printf("\nTotal %d bytes received", total);
    return 0;
}