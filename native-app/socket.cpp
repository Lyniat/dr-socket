#include "socket.h"
#include "ext.h"
#include <cstdio>
#include <cstring>
#include <memory.h>

socket_server_t socket_server;
socket_client_t socket_client;
std::vector<data_buffer_t> socket_server_received_buffer;

void socket_init(mrb_state *state){
    if (enet_initialize() != 0)
    {
        ruby_print(state, "An error occurred while initializing ENet.\n");
    }else{
        ruby_print(state, "ENet was successfully initialized.\n");
    }
}

void socket_shutdown(){
    enet_deinitialize();
    fprintf (stdout, "ENet was shutdown.\n");
}

void socket_server_create(int port, int max_clients, int num_channels){
    ENetAddress address;

    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = port;

    ENetHost * server;

    server = enet_host_create (& address        /* the address to bind the server host to */,
                               max_clients      /* allow up to 32 clients and/or outgoing connections */,
                               num_channels     /* allow up to 2 channels to be used, 0 and 1 */,
                               0                /* assume any amount of incoming bandwidth */,
                               0                /* assume any amount of outgoing bandwidth */);

    if (server == nullptr)
    {
        fprintf (stderr, "An error occurred while trying to create an ENet server host.\n");
        return;
    }

    fprintf (stdout, "ENet server host was successfully created on port %i with %i channels.\n",port, num_channels);

    socket_server = {.address = address, .server = server, .port = port, .max_clients = max_clients};
}

void socket_server_destroy(){
    enet_host_destroy(socket_server.server);
    fprintf (stdout, "ENet server was destroyed.\n");
}

void socket_client_create(int num_channels){
    ENetHost * client;
    client = enet_host_create (nullptr /* create a client host */,
                               1            /* only allow 1 outgoing connection */,
                               num_channels /* allow up 2 channels to be used, 0 and 1 */,
                               0            /* assume any amount of incoming bandwidth */,
                               0            /* assume any amount of outgoing bandwidth */);
    if (client == nullptr)
    {
        fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
        return;
    }

    fprintf (stdout, "ENet client host was successfully created with %i channels.\n", num_channels);
    socket_client = {.client = client, .server_address = "undefined", .port = -1};
}

void socket_client_destroy(){
    enet_host_destroy(socket_client.client);
    FREE((void*)socket_client.server_address);
    fprintf (stdout, "ENet client was destroyed.\n");
}

void socket_client_connect(const char* _address, int port){
    ENetAddress address;
    ENetPeer *peer;
    /* Connect to some.server.net:1234. */
    enet_address_set_host (& address, _address);
    address.port = port;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (socket_client.client, & address, 2, 0);

    socket_client.server_address = strdup(_address);
    socket_client.port = port;

    if (peer == nullptr)
    {
        fprintf (stderr, "No available peers for initiating an ENet connection to %s:%i.\n", _address, port);
        return;
    }
}

void socket_server_update(){
    ENetEvent event;
    const char* buffer;
    if(enet_host_service(socket_server.server, & event, 0) > 0){
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf ("A new client connected from %x:%u.\n",
                        event.peer -> address.host,
                        event.peer -> address.port);
                /* Store any relevant client information here. */
                event.peer -> data = (void*)"Client information";
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
                        event.packet -> dataLength,
                        event.packet -> data,
                        event.peer -> data,
                        event.channelID);
                buffer = (const char*)MALLOC(event.packet->dataLength);
                memcpy((void*)buffer, event.packet->data, event.packet->dataLength);
                socket_server_received_buffer.push_back({.buffer = buffer, .size = (int)event.packet->dataLength});
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconnected.\n", event.peer -> data);
                /* Reset the peer's client information. */
                event.peer -> data = nullptr;
        }
    }
}

void socket_client_update(){
    ENetEvent event;
    if (enet_host_service (socket_client.client, & event, 0) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        fprintf( stdout, "Connection to %s:%i succeeded.\n", socket_client.server_address, socket_client.port);
    }
}

void socket_client_send_to_server(const char* buffer, int size){
    ENetPacket * packet = enet_packet_create (buffer,
                                              size,
                                              ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(socket_client.client->peers, 0, packet);
}

void socket_client_send_str_to_server(const char* str, size_t str_len){
    ENetPacket * packet = enet_packet_create (str,
                                              str_len + 1,
                                              ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(socket_client.client->peers, 0, packet);
}