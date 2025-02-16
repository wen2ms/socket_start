#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (file_descriptor == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(file_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));

    if (ret == -1) {
        perror("bind");
        return -1;
    }

    ret = listen(file_descriptor, 128);

    if (ret == -1) {
        perror("listen");
        return -1;
    }

    struct sockaddr_in client_address;
    unsigned int client_address_len = sizeof(client_address);

    int client_file_descriptor = accept(file_descriptor, (struct sockaddr*)&client_address, &client_address_len);

    if (client_file_descriptor == -1) {
        perror("accept");
        return -1;
    }

    char ip[32];
    printf("Client IP: %s, Port: %d\n", inet_ntop(AF_INET, &client_address.sin_addr.s_addr, ip, sizeof(ip)),
           ntohs(client_address.sin_port));
        
    while (1) {
        char buffer[1024];
        int receive_data_len = recv(client_file_descriptor, buffer, sizeof(buffer), 0);

        if (receive_data_len > 0) {
            printf("client say: %s\n", buffer);

            send(client_file_descriptor, buffer, receive_data_len, 0);
        } else if (receive_data_len == 0) {
            printf("client disconnected...\n");
            break;
        } else {
            perror("recv");
            return -1;
        }
    }

    close(file_descriptor);
    close(client_file_descriptor);

    return 0;
}