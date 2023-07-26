#ifndef DR_SOCKET_ENET_H
#define DR_SOCKET_ENET_H

#include <dragonruby.h>

extern ENetHost* socket_enet_host;
extern ENetPeer* socket_enet_peer;

extern mrb_value socket_event_receive;
extern mrb_value socket_event_connect;
extern mrb_value socket_event_disconnect;
extern mrb_value socket_event_none;

void register_socket_symbols(mrb_state *state);

ENetHost* get_enet_host();
ENetPeer* get_enet_peer();

#endif