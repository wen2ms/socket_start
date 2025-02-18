#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct SocketInfo {
    struct sockaddr_in client_address;
    int client_file_descriptor;
};

struct SocketInfo socket_infos[512];

void* worker(void* arg);

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

    int socket_infos_max = sizeof(socket_infos) / sizeof(socket_infos[0]);

    for (int i = 0; i <socket_infos_max; ++i) {
        bzero(&socket_infos[i], sizeof(socket_infos[i]));

        socket_infos[i].client_file_descriptor = -1;
    }

    unsigned int client_address_len = sizeof(struct sockaddr);

    while (1) {
        struct SocketInfo* p_info;

        for (int i = 0; i < socket_infos_max; ++i) {
            if (socket_infos[i].client_file_descriptor == -1) {
                p_info = &socket_infos[i];
                break;
            }
        }

        int client_file_descriptor = accept(file_descriptor, (struct sockaddr*)&p_info->client_address, &client_address_len);
        p_info->client_file_descriptor = client_file_descriptor;

        if (client_file_descriptor == -1) {
            perror("accept");
            break;
        }

        pthread_t tid;

        pthread_create(&tid, NULL, worker, p_info);
        
        pthread_detach(tid);
    }

    close(file_descriptor);

    return 0;
}

void* worker(void* arg) {
    printf("worker id=%p...\n", pthread_self());

    struct SocketInfo* p_info = (struct SocketInfo*)arg;

    char ip[32];
    printf("Client IP: %s, Port: %d\n", inet_ntop(AF_INET, &p_info->client_address.sin_addr.s_addr, ip, sizeof(ip)),
           ntohs(p_info->client_address.sin_port));

    while (1) {
        char buffer[1024];
        int receive_data_len = recv(p_info->client_file_descriptor, buffer, sizeof(buffer), 0);

        if (receive_data_len > 0) {
            printf("client say: %s\n", buffer);

            send(p_info->client_file_descriptor, buffer, receive_data_len, 0);
        } else if (receive_data_len == 0) {
            printf("client disconnected...\n");
            break;
        } else {
            perror("recv");
            break;
        }
    }

    close(p_info->client_file_descriptor);
    p_info->client_file_descriptor = -1;

    return NULL;
}