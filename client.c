#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
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

    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr.s_addr);

    int ret = connect(file_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));

    if (ret == -1) {
        perror("connect");
        return -1;
    }
    
    int number = 0;

    while (1) {
        char buffer[1024];

        sprintf(buffer, "number=%d...\n", number++);
        send(file_descriptor, buffer, strlen(buffer) + 1, 0);
        
        memset(buffer, 0, sizeof(buffer));

        int receive_data_len = recv(file_descriptor, buffer, sizeof(buffer), 0);

        if (receive_data_len > 0) {
            printf("server say: %s\n", buffer);
        } else if (receive_data_len == 0) {
            printf("server disconnected...\n");
            break;
        } else {
            perror("recv");
            break;
        }

        sleep(1);
    }

    close(file_descriptor);

    return 0;
}