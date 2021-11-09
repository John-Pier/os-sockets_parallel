#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PORT 6969

char message_data[]={"Hello world"};
int client_socket;
struct sockaddr_in address;

int main() {
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    connect(client_socket, (struct sockaddr *)&address, sizeof(address));
    send(client_socket, message_data, strlen(message_data), 0);
    close(client_socket);
    return 0;
}
