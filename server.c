#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "thread_pool.h"

struct SocketInfo {
    struct sockaddr_in client_address;
    int client_file_descriptor;
};

struct AcceptInfo {
    ThreadPool* thread_pool;
    int server_file_descriptor;
};

void server_worker(void* arg);
void accept_connection(void* arg);

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

    ThreadPool* thread_pool = thread_pool_create(3, 8, 100);

    struct AcceptInfo* accept_info = (struct AcceptInfo*)malloc(sizeof(struct AcceptInfo));

    accept_info->thread_pool = thread_pool;
    accept_info->server_file_descriptor = file_descriptor;

    thread_pool_add_task(thread_pool, accept_connection, accept_info);

    pthread_exit(NULL);

    return 0;
}

void accept_connection(void* arg) {
    struct AcceptInfo* accept_info = (struct AcceptInfo*)arg;

    unsigned int client_address_len = sizeof(struct sockaddr_in);

    while (1) {
        struct SocketInfo* p_info;

        p_info = (struct SocketInfo*)malloc(sizeof(struct SocketInfo));

        int client_file_descriptor =
            accept(accept_info->server_file_descriptor, (struct sockaddr*)&p_info->client_address, &client_address_len);
        p_info->client_file_descriptor = client_file_descriptor;

        if (client_file_descriptor == -1) {
            perror("accept");
            break;
        }

        thread_pool_add_task(accept_info->thread_pool, server_worker, p_info);
    }

    close(accept_info->server_file_descriptor);
}

void server_worker(void* arg) {
    printf("server_worker id=%p...\n", pthread_self());

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
}