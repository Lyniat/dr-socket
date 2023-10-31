#pragma once
#ifndef DR_SOCKET_ENET_H
#define DR_SOCKET_ENET_H

#include "ruby.h"
#include <map>
#include <enet/enet.h>
#include <vector>
#include <string>

namespace lyniat::socket::enet {

    typedef struct socket_peer_t {
        ENetPeer *peer;
        bool authorized;
    } socket_peer_t;

    typedef struct socket_event_t {
        ENetPeer *peer;
        mrb_value data;
    } socket_event_t;

    extern ENetHost *socket_enet_host;

    extern std::map<uint64_t, socket_peer_t> socket_enet_peers;

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

    void parse_address(mrb_state *l, const char *addr_str, ENetAddress *address);

    size_t find_peer_index(mrb_state *l, ENetHost *enet_host, ENetPeer *peer);

    uint64_t compute_peer_key(ENetPeer *peer);

    uint64_t get_peer_key(ENetPeer *peer);

    mrb_value event_to_hash(mrb_state *l, ENetEvent *event);

    ENetPacket *read_packet(mrb_state *l, int idx, enet_uint8 *channel_id);

    mrb_value linked_version(mrb_state *l, mrb_value self);

    mrb_value host_check_events(mrb_state *l, mrb_value self);

    mrb_value host_compress_with_range_coder(mrb_state *l);

    mrb_value host_flush(mrb_state *l, mrb_value self);

    mrb_value host_broadcast(mrb_state *l, mrb_value self);

    mrb_value host_channel_limit(mrb_state *l, mrb_value self);

    mrb_value host_bandwidth_limit(mrb_state *l, mrb_value self);

    mrb_value host_get_socket_address(mrb_state *l, mrb_value self);

    mrb_value host_total_sent_data(mrb_state *l, mrb_value self);

    mrb_value host_total_received_data(mrb_state *l, mrb_value self);

    mrb_value host_service_time(mrb_state *l, mrb_value self);

    mrb_value host_peer_count(mrb_state *l, mrb_value self);

    mrb_value host_get_peer(mrb_state *l, mrb_value self);

    mrb_value host_gc(mrb_state *l);

    mrb_value peer_tostring(mrb_state *l);

    mrb_value peer_ping(mrb_state *l, mrb_value self);

    mrb_value peer_throttle_configure(mrb_state *l, mrb_value self);

    int peer_round_trip_time(mrb_state *l);

    int peer_last_round_trip_time(mrb_state *l);

    mrb_value peer_ping_interval(mrb_state *l, mrb_value self);

    mrb_value peer_timeout(mrb_state *l, mrb_value self);

    mrb_value peer_disconnect(mrb_state *l, mrb_value self);

    mrb_value peer_disconnect_now(mrb_state *l, mrb_value self);

    mrb_value peer_disconnect_later(mrb_state *l, mrb_value self);

    mrb_value peer_index(mrb_state *l, mrb_value self);

    mrb_value peer_state(mrb_state *l, mrb_value self);

    mrb_value peer_connect_id(mrb_state *l, mrb_value self);

    mrb_value peer_receive(mrb_state *l, mrb_value self);

    mrb_value peer_send(mrb_state *l, mrb_value self);

    mrb_int get_peer_id(mrb_state *state, mrb_value self);

    class DRPeer {
    public:
        DRPeer(mrb_state *state, bool is_host, mrb_int port, bool only_local);
        ~DRPeer();

        void Connect(mrb_state *state, std::string address);
        void Disconnect(mrb_state *state, mrb_int peer_to_disconnect);
        mrb_value GetNextEvent(mrb_state *state);
        void Send(mrb_state *state, mrb_value data, mrb_int receiver);
        bool IsConnected();
    private:
        bool m_is_host;
        bool m_only_local;
        bool m_is_connected;
        std::string m_address;
        mrb_int m_port;
        ENetHost *m_host;
        ENetPeer *m_server;
    };

    extern std::map<mrb_int, DRPeer*> dr_peers;
    extern mrb_int peer_counter;

    typedef struct DRPeerStruct {
        mrb_int internal_peer_id;
    } DRPeerStruct;

    extern struct mrb_data_type dr_peer_struct;
    extern mrb_value dr_peer_initialize(mrb_state *state, mrb_value self);

}

#endif