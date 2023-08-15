#pragma once
#ifndef DR_SOCKET_ENET_H
#define DR_SOCKET_ENET_H

#include "ruby.h"
#include <map>
#include "lua.h"
#include <enet/enet.h>

namespace lyniat::socket::enet {

    typedef struct socket_event_t {
        ENetPeer *peer;
        mrb_value data;
    } socket_event_t;

    extern ENetHost *socket_enet_host;
    extern ENetPeer *socket_enet_peer;

    extern std::map<uintptr_t, ENetPeer *> socket_enet_peers;
    extern std::vector<socket_event_t> socket_enet_events;

    extern mrb_value socket_event_receive;
    extern mrb_value socket_event_connect;
    extern mrb_value socket_event_disconnect;
    extern mrb_value socket_event_none;

    extern mrb_value socket_state_disconnected;
    extern mrb_value socket_state_connecting;
    extern mrb_value socket_state_acknowledging_connect;
    extern mrb_value socket_state_connection_pending;
    extern mrb_value socket_state_connection_succeeded;
    extern mrb_value socket_state_connected;
    extern mrb_value socket_state_disconnect_later;
    extern mrb_value socket_state_disconnecting;
    extern mrb_value socket_state_acknowledging_disconnect;
    extern mrb_value socket_state_zombie;
    extern mrb_value socket_state_unknown;

    extern mrb_sym socket_order_flag_reliable;
    extern mrb_sym socket_order_flag_unsequenced;
    extern mrb_sym socket_order_flag_unreliable;


    void init_enet_bindings();

    void register_socket_symbols(mrb_state *state);

    void socket_open_enet(mrb_state *state);

    ENetHost *get_enet_host();

    ENetPeer *get_enet_peer();

    void parse_address(lua_State *l, const char *addr_str, ENetAddress *address);

    size_t find_peer_index(lua_State *l, ENetHost *enet_host, ENetPeer *peer);

    uintptr_t compute_peer_key(lua_State *L, ENetPeer *peer);

    void push_peer_key(lua_State *L, uintptr_t key);

    uintptr_t push_peer(lua_State *l, ENetPeer *peer);

    mrb_value push_event(lua_State *l, ENetEvent *event);

    ENetPacket *read_packet(lua_State *l, int idx, enet_uint8 *channel_id);

    mrb_value host_create(lua_State *l, mrb_value self);

    mrb_value linked_version(lua_State *l, mrb_value self);

    mrb_value host_service(lua_State *l, mrb_value self);

    mrb_value host_check_events(lua_State *l, mrb_value self);

    mrb_value host_compress_with_range_coder(lua_State *l);

    mrb_value host_connect(lua_State *l, mrb_value self);

    mrb_value host_flush(lua_State *l, mrb_value self);

    mrb_value host_broadcast(lua_State *l, mrb_value self);

    mrb_value host_channel_limit(lua_State *l, mrb_value self);

    mrb_value host_bandwidth_limit(lua_State *l, mrb_value self);

    mrb_value host_get_socket_address(lua_State *l, mrb_value self);

    mrb_value host_total_sent_data(lua_State *l, mrb_value self);

    mrb_value host_total_received_data(lua_State *l, mrb_value self);

    mrb_value host_service_time(lua_State *l, mrb_value self);

    mrb_value host_peer_count(lua_State *l, mrb_value self);

    mrb_value host_get_peer(lua_State *l, mrb_value self);

    mrb_value host_gc(lua_State *l);

    mrb_value peer_tostring(lua_State *l);

    mrb_value peer_ping(lua_State *l, mrb_value self);

    mrb_value peer_throttle_configure(lua_State *l, mrb_value self);

    int peer_round_trip_time(lua_State *l);

    int peer_last_round_trip_time(lua_State *l);

    mrb_value peer_ping_interval(lua_State *l, mrb_value self);

    mrb_value peer_timeout(lua_State *l, mrb_value self);

    mrb_value peer_disconnect(lua_State *l, mrb_value self);

    mrb_value peer_disconnect_now(lua_State *l, mrb_value self);

    mrb_value peer_disconnect_later(lua_State *l, mrb_value self);

    mrb_value peer_index(lua_State *l, mrb_value self);

    mrb_value peer_state(lua_State *l, mrb_value self);

    mrb_value peer_connect_id(lua_State *l, mrb_value self);

    mrb_value peer_receive(lua_State *l, mrb_value self);

    mrb_value peer_send(lua_State *l, mrb_value self);

}

#endif