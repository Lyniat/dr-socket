#ifndef DR_SOCKET_SOCKET_H
#define DR_SOCKET_SOCKET_H

#include <enet/enet.h>
#include "serialize.h"
#include <vector>

typedef struct socket_server_t{
    ENetAddress address;
    ENetHost * server;
    int port;
    int max_clients;
} socket_server_t;

typedef struct socket_client_t{
    ENetHost * client;
    const char* server_address;
    int port;
} socket_client_t;

typedef struct data_buffer_t{
    const char* buffer;
    int size;
} data_buffer_t;

extern socket_server_t socket_server;
extern socket_client_t socket_client;
extern std::vector<data_buffer_t> socket_server_received_buffer;

void socket_init(mrb_state *state);
void socket_shutdown();
void socket_server_create(int port, int max_clients, int num_channels);
void socket_server_destroy();
void socket_client_create(int num_channels);
void socket_client_destroy();
void socket_client_connect(const char* _address, int port);
void socket_server_update();
void socket_client_update();
void socket_client_send_str_to_server(const char* str, size_t str_len);
void socket_client_send_to_server(const char* buffer, int size);

#endif