// based on https://github.com/love2d/love/blob/main/src/libraries/enet/enet.cpp
// with following license

/**
 *
 * Copyright (C) 2014 by Leaf Corcoran
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// from original löve code
// #ifdef _WIN32
// #define NOMINMAX
// #endif

#include <cstdlib>
#include <cstring>
#include <cmath>

#include <cstdio>
#include <algorithm>

#include <enet/enet.h>
#include "lyniat/memory.h"
#include "enet.h"
#include "api.h"

#include "help.h"
#include "serialize.h"
#include "print.h"
#include "buffer.h"
#include "test.h"
#include "file.h"
#include "socket.rb.h"

/*
#define check_host(state, idx)\
	*(ENetHost**)luaL_checkudata(state, idx, "enet_host")

#define check_peer(state, idx)\
	*(ENetPeer**)luaL_checkudata(state, idx, "enet_peer")
*/

using namespace lyniat::socket;
namespace lyniat::socket::enet {

#ifdef DEBUG
#define define_debug_function(f, name, args) \
    API->mrb_define_module_function(state, module, name, f, MRB_ARGS_REQ(args))
#else
#define define_debug_function(f, name, args) \
    API->mrb_define_module_function(state, module, name, {[](mrb_state *mrb, mrb_value self) { \
        print::print(mrb, print::PRINT_ERROR, "Function {} is only available in Debug build.", #f); \
        return mrb_nil_value(); \
    }}, MRB_ARGS_REQ(args))
#endif
#define define_function(f, args) \
    API->mrb_define_module_function(state, module, #f, f, MRB_ARGS_REQ(args))

#define define_function_name(f, args, name) \
    API->mrb_define_module_function(state, module, name, f, MRB_ARGS_REQ(args))

#define undefine_function(f, args) \
    API->mrb_define_module_function(state, module, #f, {[](mrb_state *mrb, mrb_value self) { \
        print::print(mrb, print::PRINT_ERROR, "Function {} is currently not implemented. Sorry :(", #f); \
        return mrb_nil_value(); \
    }}, MRB_ARGS_REQ(args))

#define ENET_ALIGNOF(x) alignof(x)

mrb_data_type dr_peer_struct;

std::map<mrb_int, DRPeer*> dr_peers;
mrb_int peer_counter = 0;

ENetHost* socket_enet_host;

std::map<uint64_t, socket_peer_t> socket_enet_peers;

mrb_value socket_event_receive;
mrb_value socket_event_connect;
mrb_value socket_event_disconnect;
mrb_value socket_event_none;

mrb_value socket_state_disconnected;
mrb_value socket_state_connecting;
mrb_value socket_state_acknowledging_connect;
mrb_value socket_state_connection_pending;
mrb_value socket_state_connection_succeeded;
mrb_value socket_state_connected;
mrb_value socket_state_disconnect_later;
mrb_value socket_state_disconnecting;
mrb_value socket_state_acknowledging_disconnect;
mrb_value socket_state_zombie;
mrb_value socket_state_unknown;

mrb_sym socket_order_flag_reliable;
mrb_sym socket_order_flag_unsequenced;
mrb_sym socket_order_flag_unreliable;

mrb_sym socket_pass_to_server;
mrb_sym socket_pass_to_all_other_clients;
mrb_sym socket_pass_to_self;

constexpr const char* str_event_type = "type";
constexpr const char* str_peer = "peer";
constexpr const char* str_channel = "channel";
constexpr const char* str_data = "data";
constexpr const char* str_flag = "flag";

constexpr const enet_uint8 DEFAULT_CHANNEL = 8;

ENetHost* get_enet_host(){
    return socket_enet_host;
}

ENetPeer* get_enet_peer(uint64_t id){
    auto peer = socket_enet_peers[id];
    if(peer.authorized || true){ //TODO: add real check
        return peer.peer;
    }
    return nullptr;
}

void init_enet_bindings(){

}

void register_socket_symbols(mrb_state *mrb){
    socket_event_receive = API->mrb_symbol_value(cext_sym(mrb, "s_event_receive"));
    socket_event_connect = API->mrb_symbol_value(cext_sym(mrb, "s_event_connect"));
    socket_event_disconnect = API->mrb_symbol_value(cext_sym(mrb, "s_event_disconnect"));
    socket_event_none = API->mrb_symbol_value(cext_sym(mrb, "s_event_none"));

    socket_state_disconnected = API->mrb_symbol_value(cext_sym(mrb, "s_state_disconnected"));
    socket_state_connecting = API->mrb_symbol_value(cext_sym(mrb, "s_state_connecting"));
    socket_state_acknowledging_connect = API->mrb_symbol_value(cext_sym(mrb, "s_state_acknowledging_connect"));
    socket_state_connection_pending = API->mrb_symbol_value(cext_sym(mrb, "s_state_connection_pending"));
    socket_state_connection_succeeded = API->mrb_symbol_value(cext_sym(mrb, "s_state_connection_succeeded"));
    socket_state_connected = API->mrb_symbol_value(cext_sym(mrb, "s_state_connected"));
    socket_state_disconnect_later = API->mrb_symbol_value(cext_sym(mrb, "s_state_disconnect_later"));
    socket_state_disconnecting = API->mrb_symbol_value(cext_sym(mrb, "s_state_disconnecting"));
    socket_state_acknowledging_disconnect = API->mrb_symbol_value(cext_sym(mrb, "s_state_acknowledging_disconnect"));
    socket_state_zombie = API->mrb_symbol_value(cext_sym(mrb, "s_state_zombie"));
    socket_state_unknown = API->mrb_symbol_value(cext_sym(mrb, "s_state_unknown"));

    socket_order_flag_reliable = cext_sym(mrb, "s_order_reliable");
    socket_order_flag_unsequenced = cext_sym(mrb, "s_order_unsequenced");
    socket_order_flag_unreliable = cext_sym(mrb, "s_order_unreliable");

    socket_pass_to_server = cext_sym(mrb, "s_pass_to_server");
    socket_pass_to_all_other_clients = cext_sym(mrb, "s_pass_to_other_clients");
    socket_pass_to_self = cext_sym(mrb, "s_pass_to_self");
}

/**
 * Parse address string, eg:
 *	*:5959
 *	127.0.0.1:*
 *	website.com:8080
 */
bool parse_address(const char *addr_str, ENetAddress *address, const char *error) {
    int host_i = 0, port_i = 0;
    char host_str[128] = {0};
    char port_str[32] = {0};
    int scanning_port = 0;

    char *c = (char *)addr_str;

    while (*c != 0) {
        if (host_i >= 128 || port_i >= 32 ){
            error = "Hostname too long";
            return true;
        }
        if (scanning_port) {
            port_str[port_i++] = *c;
        } else {
            if (*c == ':') {
                scanning_port = 1;
            } else {
                host_str[host_i++] = *c;
            }
        }
        c++;
    }
    host_str[host_i] = '\0';
    port_str[port_i] = '\0';

    if (host_i == 0){
        error = "Failed to parse address";
        return true;
    }
    if (port_i == 0){
        error = "Missing port in address";
        return true;
    }

    if (strcmp("*", host_str) == 0) {
        address->host = ENET_HOST_ANY;
    } else {
        if (enet_address_set_host(address, host_str) != 0) {
            error = "Failed to resolve host name";
            return true;
        }
    }

    if (strcmp("*", port_str) == 0) {
        address->port = ENET_PORT_ANY;
    } else {
        address->port = atoi(port_str);
    }

        return false;
}

/**
 * Find the index of a given peer for which we only have the pointer.
 */
size_t find_peer_index(mrb_state *state, ENetHost *enet_host, ENetPeer *peer) {
    size_t peer_index;
    for (peer_index = 0; peer_index < enet_host->peerCount; peer_index++) {
        if (peer == &(enet_host->peers[peer_index]))
            return peer_index;
    }

    print::print (state, print::PRINT_ERROR, "enet: could not find peer id!");

    return peer_index;
}

uint64_t compute_peer_key(ENetPeer *peer)
{
    return (uint64_t)peer->address.host << 32 | peer->address.port;
}

uint64_t get_peer_key(ENetPeer *peer) {
    uint64_t key = compute_peer_key(peer);

    //if(!socket_enet_peers.count(key)){
    socket_enet_peers[key] = {peer, false};
    //}

    return key;
}

mrb_value event_to_hash(mrb_state *state, simplified_enet_event_t *event) {
    auto hash = API->mrb_hash_new(state);

    mrb_value data;
    buffer::BinaryBuffer *buffer;

    cext_hash_set_ksym(state, hash, str_peer, API->mrb_int_value(state, (mrb_int)event->peer_key));

    switch (event->type) {
        case ENET_EVENT_TYPE_CONNECT:
            cext_hash_set_ksym(state, hash, str_event_type, socket_event_connect);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            cext_hash_set_ksym(state, hash, str_event_type, socket_event_disconnect);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            cext_hash_set_ksym(state, hash, str_event_type, socket_event_receive);
            cext_hash_set_ksym(state, hash, str_channel, API->mrb_int_value(state, event->channel_id));

            buffer = new buffer::BinaryBuffer(event->data, event->data_length, false);
            data = serialize::deserialize_data(buffer, state);

            cext_hash_set_ksym(state, hash, str_data, data);

            delete buffer;
            break;
        case ENET_EVENT_TYPE_NONE:
            cext_hash_set_ksym(state, hash, str_event_type, socket_event_none);
            break;
    }

    return hash;
}

/**
 * Read a packet off the stack as a string
 * idx is position of string
 */
ENetPacket *read_packet(mrb_state *state, int idx, enet_uint8 *channel_id) {
    // TODO: add
        return nullptr;
}

mrb_value linked_version(mrb_state *state, mrb_value self) {
    char* buffer;
    size_t size;
    size = snprintf(nullptr, 0, "%d.%d.%d",
                    ENET_VERSION_GET_MAJOR(enet_linked_version()),
                    ENET_VERSION_GET_MINOR(enet_linked_version()),
                    ENET_VERSION_GET_PATCH(enet_linked_version()));

    buffer = (char *)MALLOC(size + 1);
    snprintf(buffer, size + 1, "%d.%d.%d",
             ENET_VERSION_GET_MAJOR(enet_linked_version()),
             ENET_VERSION_GET_MINOR(enet_linked_version()),
             ENET_VERSION_GET_PATCH(enet_linked_version()));

    auto result = API->mrb_str_new(state, buffer, size);
    FREE(buffer);
    return result;
}

/**
 * Enables an adaptive order-2 PPM range coder for the transmitted data of
 * all peers.
 */
 // TODO: do wee need this?
 /*
mrb_value host_compress_with_range_coder(mrb_state *state) {
    ENetHost *host = check_host(state, 1);
    if (!host) {
        return print::print(state, "Tried to index a nil host!");
    }

    int result = enet_host_compress_with_range_coder (host);
    if (result == 0) {
        lua_pushboolean (state, 1);
    } else {
        lua_pushboolean (state, 0);
    }

    return CEXT_INT(state, 1);
}
*/

mrb_value host_flush(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }
    enet_host_flush(host);
    return mrb_nil_value();
}

mrb_value host_broadcast(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }

    enet_uint8 channel_id;
    ENetPacket *packet = read_packet(state, 2, &channel_id);
    enet_host_broadcast(host, channel_id, packet);
    return mrb_nil_value();
}

// Args: limit:number
mrb_value host_channel_limit(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }
    mrb_int limit;
        API->mrb_get_args(state, "i", &limit);
    enet_host_channel_limit(host, limit);
    return mrb_nil_value();
}

// https://leafo.net/lua-enet/#hostbandwidth_limitincoming_outgoing
mrb_value host_bandwidth_limit(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }
    enet_uint32 in_bandwidth, out_bandwidth;

        API->mrb_get_args(state, "ii", &in_bandwidth, &out_bandwidth);

    enet_host_bandwidth_limit(host, in_bandwidth, out_bandwidth);
    return mrb_nil_value();
}

mrb_value host_get_socket_address(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }
    ENetAddress address;
    enet_socket_get_address (host->socket, &address);

    char* buffer;
    size_t size;
    size = snprintf(nullptr, 0, "%d.%d.%d.%d:%d",
                  ((address.host) & 0xFF),
                  ((address.host >> 8) & 0xFF),
                  ((address.host >> 16) & 0xFF),
                  (address.host >> 24& 0xFF),
                  address.port);
    buffer = (char *)MALLOC(size + 1);
    snprintf(buffer, size + 1, "%d.%d.%d.%d:%d",
                 ((address.host) & 0xFF),
                 ((address.host >> 8) & 0xFF),
                 ((address.host >> 16) & 0xFF),
                 (address.host >> 24& 0xFF),
                 address.port);

    auto result = API->mrb_str_new(state, buffer, size);
    //TODO: free buffer?
    return result;
}

mrb_value host_total_sent_data(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(state, host->totalSentData);
    return result;
}

mrb_value host_total_received_data(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(state, host->totalReceivedData);
    return result;
}

mrb_value host_service_time(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(state, host->serviceTime);
    return result;
}

mrb_value host_peer_count(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(state, host->peerCount);
    return result;
}

// https://leafo.net/lua-enet/#hostget_peerindex
// TODO: !
mrb_value host_get_peer(mrb_state *state, mrb_value self) {
    ENetHost *host = get_enet_host();
    if (!host) {
        return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
    }

    mrb_int peer_index;
        API->mrb_get_args(state, "i", &peer_index);

    if (peer_index < 0 || ((size_t) peer_index) >= host->peerCount) {
        //luaL_argerror (state, 2, "Invalid peer index");
        return print::print(state, print::PRINT_ERROR, "Invalid peer index");
    }

    ENetPeer *peer = &(host->peers[peer_index]);

        get_peer_key(peer);
    //return CEXT_INT(state, 1);
    return mrb_nil_value();
}

mrb_value host_gc(mrb_state *state) {
    if (socket_enet_host) {
        enet_host_destroy(socket_enet_host);
    }
    socket_enet_host = nullptr;
    return mrb_nil_value();
}

mrb_value peer_ping(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);
    enet_peer_ping(peer);
    return mrb_nil_value();
}

// https://leafo.net/lua-enet/#peerthrottle_configureinterval_acceleration_deceleration
mrb_value peer_throttle_configure(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    enet_uint32 intervastate, acceleration, deceleration;
    API->mrb_get_args(state, "iii", &intervastate, &acceleration, &deceleration);

    enet_peer_throttle_configure(peer, intervastate, acceleration, deceleration);
    return mrb_nil_value();
}

// TODO: implement this
/*
int peer_round_trip_time(mrb_state *state) {
    ENetPeer *peer = check_peer(state, 1);

    if (lua_gettop(l) > 1) {
        enet_uint32 round_trip_time = (int) luaL_checknumber(state, 2);
        peer->roundTripTime = round_trip_time;
    }

    lua_pushinteger (state, peer->roundTripTime);

    return 1;
}
*/

// TODO: implement this
/*
int peer_last_round_trip_time(mrb_state *state) {
    ENetPeer *peer = check_peer(state, 1);

    if (lua_gettop(l) > 1) {
        enet_uint32 round_trip_time = (int) luaL_checknumber(state, 2);
        peer->lastRoundTripTime = round_trip_time;
    }
    lua_pushinteger (state, peer->lastRoundTripTime);

    return 1;
}
 */

// https://leafo.net/lua-enet/#peerping_intervalinterval
mrb_value peer_ping_interval(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    enet_uint32 interval;
        API->mrb_get_args(state, "i", &interval);

    enet_peer_ping_interval (peer, interval);

    //return CEXT_INT(state, 1);
    return mrb_nil_value();
}

mrb_value peer_timeout(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    enet_uint32 timeout_limit, timeout_minimum, timeout_maximum;

    API->mrb_get_args(state, "iii", &timeout_limit, &timeout_minimum, &timeout_maximum);

    enet_peer_timeout (peer, timeout_limit, timeout_minimum, timeout_maximum);

    auto result = API->mrb_ary_new(state);
    API->mrb_ary_push(state, result, API->mrb_int_value(state, peer->timeoutLimit));
    API->mrb_ary_push(state, result, API->mrb_int_value(state, peer->timeoutMinimum));
    API->mrb_ary_push(state, result, API->mrb_int_value(state, peer->timeoutMaximum));

    return result;
}

//TODO: accept optional value like in https://leafo.net/lua-enet/#peerdisconnectdata
mrb_value peer_disconnect(mrb_state *state, mrb_value self) {
    mrb_int peer_id;
    API->mrb_get_args(state, "i", &peer_id);
    ENetPeer *peer = get_enet_peer(peer_id);

    enet_uint32 data = 0;
    enet_peer_disconnect(peer, data);
    return mrb_nil_value();
}

//TODO: accept optional value like in https://leafo.net/lua-enet/#peerdisconnect_nowdata
mrb_value peer_disconnect_now(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    //enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(state, 2) : 0;
    enet_uint32 data = 0;
    enet_peer_disconnect_now(peer, data);
    return mrb_nil_value();
}

//TODO: accept optional value like in https://leafo.net/lua-enet/#peerdisconnect_laterdata
mrb_value peer_disconnect_later(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    //enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(state, 2) : 0;
    enet_uint32 data = 0;
    enet_peer_disconnect_later(peer, data);
    return mrb_nil_value();
}

mrb_value peer_index(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    size_t peer_index = find_peer_index (state, peer->host, peer);

    return API->mrb_int_value(state, (mrb_int)peer_index);
}

mrb_value peer_state(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    switch (peer->state) {
        case (ENET_PEER_STATE_DISCONNECTED):
            return socket_state_disconnected;
        case (ENET_PEER_STATE_CONNECTING):
            return socket_state_connecting;
        case (ENET_PEER_STATE_ACKNOWLEDGING_CONNECT):
            return socket_state_acknowledging_connect;
        case (ENET_PEER_STATE_CONNECTION_PENDING):
            return socket_state_connection_pending;
        case (ENET_PEER_STATE_CONNECTION_SUCCEEDED):
            return socket_state_connection_succeeded;
        case (ENET_PEER_STATE_CONNECTED):
            return  socket_state_connected;
        case (ENET_PEER_STATE_DISCONNECT_LATER):
            return  socket_state_disconnect_later;
        case (ENET_PEER_STATE_DISCONNECTING):
            return  socket_state_disconnecting;
        case (ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT):
            return  socket_state_acknowledging_disconnect;
        case (ENET_PEER_STATE_ZOMBIE):
            return socket_state_zombie;
        default:
            return socket_state_unknown;
    }
}

mrb_value peer_connect_id(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);

    //lua_pushinteger (state, peer->connectID);
    return API->mrb_int_value(state, peer->connectID);
}


mrb_value peer_reset(mrb_state *state, mrb_value self) {
    ENetPeer *peer = get_enet_peer(1);
    enet_peer_reset(peer);

    return mrb_nil_value();
}

//TODO: channel_id?
/*
mrb_value peer_receive(mrb_state *state, mrb_value self) {
    ENetPeer *peer = check_peer(state, 1);
    ENetPacket *packet;
    enet_uint8 channel_id = 0;

    if (lua_gettop(l) > 1) {
        channel_id = (int) luaL_checknumber(state, 2);
    }

    packet = enet_peer_receive(peer, &channel_id);
    if (packet == nullptr){
        return mrb_nil_value();
    }

    lua_pushlstring(state, (const char *)packet->data, packet->dataLength);
    lua_pushinteger(state, channel_id);

    enet_packet_destroy(packet);
    return 2;
}
*/


/**
 * Send a lua string to a peer
 * Args:
 *	packet data, string
 *	channel id
 *	flags ["reliable", nil]
 *
 */
mrb_value peer_send(mrb_state *state, mrb_value self) {
    mrb_int peer_id;
    mrb_value h;
    API->mrb_get_args(state, "iH", &peer_id, &h);

    auto data = cext_hash_get_save_hash(state, h, str_data);
    auto sym_flag = cext_hash_get_sym_default(state, h, str_flag, socket_order_flag_reliable);
    auto channel_id = cext_hash_get_int_default(state, h, str_channel, 0);

    auto flag = ENET_PACKET_FLAG_RELIABLE;

    if(sym_flag == socket_order_flag_unreliable){
        flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
    }else if(sym_flag == socket_order_flag_unsequenced){
        flag = ENET_PACKET_FLAG_UNSEQUENCED;
    }

    auto buffer = new buffer::BinaryBuffer();
    serialize::serialize_data(buffer, state, data);

    ENetPacket * packet = enet_packet_create (buffer->Data(),
                                              buffer->Size(),
                                              flag);

    ENetPeer *peer = get_enet_peer(peer_id);

    int ret = enet_peer_send(peer, channel_id, packet);
    if (ret < 0) {
        enet_packet_destroy(packet);
    }

    delete buffer;

    return API->mrb_int_value(state, ret);
}

void socket_open_enet(mrb_state* state) {
    struct RClass *FFI = API->mrb_module_get(state, "FFI");
    //struct RClass *module = API->mrb_module_get_under(state, FFI, "DRSocket");
    //struct RClass *module = API->mrb_module_get_under(state, module_socket, "Raw");
    struct RClass *module = API->mrb_define_module_under(state, FFI, "DRSocket");

    struct RClass *class_dr_peer;

    class_dr_peer = API->mrb_define_class_under(state, module, "Peer", state->object_class);
    MRB_SET_INSTANCE_TT(class_dr_peer, MRB_TT_DATA);
    API->mrb_define_method(state, class_dr_peer, "initialize", dr_peer_initialize, MRB_ARGS_REQ(3));

    API->mrb_define_method(state, class_dr_peer, "next_event", {[](mrb_state *state, mrb_value self) {
        auto peer_id = get_peer_id(state, self);
        auto peer = dr_peers[peer_id];
        auto event = peer->GetNextEvent(state);
        return event;
    }}, MRB_ARGS_NONE());

    API->mrb_define_method(state, class_dr_peer, "send", {[](mrb_state *state, mrb_value self) {
        mrb_value data;
        mrb_int receiver;
        API->mrb_get_args(state, "iH", &receiver, &data);
        auto peer_id = get_peer_id(state, self);
        auto peer = dr_peers[peer_id];
        peer->Send(state, data, receiver);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(2));

    API->mrb_define_method(state, class_dr_peer, "connect", {[](mrb_state *state, mrb_value self) {
        mrb_value rb_address;
        mrb_int port;
        API->mrb_get_args(state, "Si", &rb_address, &port);
        auto peer_id = get_peer_id(state, self);
        auto peer = dr_peers[peer_id];
        auto address = fmt::format("{}:{}", API->mrb_string_cstr(state, rb_address), port);
        peer->Connect(state, address);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(2));

    API->mrb_define_method(state, class_dr_peer, "disconnect", {[](mrb_state *state, mrb_value self) {
        mrb_int peer_to_disconnect;
        API->mrb_get_args(state, "i", &peer_to_disconnect);
        auto peer_id = get_peer_id(state, self);
        auto peer = dr_peers[peer_id];
        peer->Disconnect(state, peer_to_disconnect);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(1));

    API->mrb_define_method(state, class_dr_peer, "connected?", {[](mrb_state *state, mrb_value self) {
        auto peer_id = get_peer_id(state, self);
        auto peer = dr_peers[peer_id];
        auto is_connected = peer->IsConnected();
        return mrb_bool_value(is_connected);
    }}, MRB_ARGS_REQ(0));

    API->mrb_load_string(state, ruby_socket_code);

    enet_initialize();
    //atexit(enet_deinitialize); TODO: use this

    API->mrb_define_module_function(state, module, "__peer_initialize", {[](mrb_state *state, mrb_value self) {
        mrb_int is_host, port, only_local;
        API->mrb_get_args(state, "iii", &is_host, &port, &only_local);
        auto peer = new DRPeer(state, is_host != 0, port, only_local != 0);
        dr_peers[peer_counter] = peer;
        auto to_return = peer_counter;
        peer_counter++;
        return API->mrb_int_value(state, to_return);
    }}, MRB_ARGS_REQ(3));

    API->mrb_define_module_function(state, module, "__get_next_event", {[](mrb_state *state, mrb_value self) {
        mrb_int peer_id;
        API->mrb_get_args(state, "i", &peer_id);
        auto peer = dr_peers[peer_id];
        auto event = peer->GetNextEvent(state);
        return event;
    }}, MRB_ARGS_REQ(1));

    API->mrb_define_module_function(state, module, "__send", {[](mrb_state *state, mrb_value self) {
        mrb_int peer_id;
        mrb_value data;
        mrb_int receiver;
        API->mrb_get_args(state, "iHi", &peer_id, &data, &receiver);
        auto peer = dr_peers[peer_id];
        peer->Send(state, data, receiver);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(3));

    API->mrb_define_module_function(state, module, "__connect", {[](mrb_state *state, mrb_value self) {
        mrb_int peer_id;
        mrb_value rb_address;
        API->mrb_get_args(state, "iS", &peer_id, &rb_address);
        auto peer = dr_peers[peer_id];
        peer->Connect(state, API->mrb_string_cstr(state, rb_address));
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(2));

    // debug
    API->mrb_define_module_function(state, module, "__debug_save", file::debug_serialized_to_file, MRB_ARGS_REQ(1));

    API->mrb_define_module_function(state, module, "get_build_info", {[](mrb_state *mrb, mrb_value self) {
        auto enet_version = linked_version(mrb, self);
        auto result = API->mrb_hash_new(mrb);
        cext_hash_set_kstr(mrb, result, "enet", enet_version);

        auto meta_platform = (const char *) META_PLATFORM;
        auto meta_type = (const char *) META_TYPE;
        auto meta_git_hash = (const char *) META_GIT_HASH;
        auto meta_git_branch = (const char *) META_GIT_BRANCH;
        auto meta_compiler_id = (const char *) META_COMPILER_ID;
        auto meta_compiler_version = (const char *) META_COMPILER_VERSION;

        auto build_information = API->mrb_hash_new(mrb);
        cext_hash_set_kstr(mrb, build_information, "target_platform", API->mrb_str_new_cstr(mrb, meta_platform));
        cext_hash_set_kstr(mrb, build_information, "build_type", API->mrb_str_new_cstr(mrb, meta_type));
        cext_hash_set_kstr(mrb, build_information, "git_hash", API->mrb_str_new_cstr(mrb, meta_git_hash));
        cext_hash_set_kstr(mrb, build_information, "git_branch", API->mrb_str_new_cstr(mrb, meta_git_branch));

        auto compiler = API->mrb_hash_new(mrb);
        cext_hash_set_kstr(mrb, compiler, "id", API->mrb_str_new_cstr(mrb, meta_compiler_id));
        cext_hash_set_kstr(mrb, compiler, "version", API->mrb_str_new_cstr(mrb, meta_compiler_version));

        cext_hash_set_kstr(mrb, build_information, "compiler", compiler);

        cext_hash_set_kstr(mrb, result, "build_information", build_information);

        return result;
    }}, MRB_ARGS_REQ(0));

    API->mrb_define_module_function(state, module, "check_allocated_memory",
                                        {[](mrb_state *mrb, mrb_value self) {
#ifdef DEBUG
                                            auto str = lyniat_memory_check_allocated_memory();
                                            return API->mrb_str_new_cstr(mrb, str);
#else
                                            print::print(mrb, "check_allocated_memory is only available in a Debug build.");
                                            return mrb_nil_value();
#endif
                                        }}, MRB_ARGS_REQ(0));

    API->mrb_define_module_function(state, module, "__free_cycle_memory",
                                        {[](mrb_state *mrb, mrb_value self) {
                                            FREE_CYCLE
                                            return mrb_nil_value();
                                        }}, MRB_ARGS_REQ(0));

    register_test_functions(state, module);
}

    DRPeer::DRPeer(mrb_state *state, bool is_host, mrb_int port, bool only_local){
        m_is_host = is_host;
        m_port = port;
        m_only_local = only_local;

        size_t peer_count = 64, channel_count = 1;
        enet_uint32 in_bandwidth = 0, out_bandwidth = 0;

        ENetAddress e_address;
        if(m_is_host){
            if(only_local){
                const char *error;
                if(parse_address(fmt::format("{}:{}", "localhost", m_port).c_str(), &e_address, error)){
                    print::print(state, print::PRINT_ERROR, error);
                    return;
                }
            }else{
                e_address.host = ENET_HOST_ANY;
                e_address.port = port;
            }
        }

        m_host = enet_host_create(m_is_host? &e_address : nullptr, peer_count,
                                channel_count, in_bandwidth, out_bandwidth);

        if (m_host == nullptr) {
            print::print(state, print::PRINT_ERROR, "enet: failed to create host (already listening?)");
            return;
        }
}
    DRPeer::~DRPeer(){


}
    void DRPeer::Connect(mrb_state *state, std::string address){
        if(m_is_host){
            print::print(state, print::PRINT_ERROR, "Server is not allowed to explicitly connect to a client!");
            return;
        }

        if (!m_host) {
            print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
            return;
        }
        ENetAddress e_address;

        enet_uint32 data = 0;
        size_t channel_count = 1;

        const char* error;
        if(parse_address(address.c_str(), &e_address, error)){
            print::print(state, print::PRINT_ERROR, error);
            return;
        }

        m_server = enet_host_connect(m_host, &e_address, channel_count, data);

        if (m_server == nullptr) {
            print::print(state, print::PRINT_ERROR, "Failed to create peer");
            return;
        }
}

    void DRPeer::Disconnect(mrb_state *state, mrb_int peer_to_disconnect){
        ENetPeer *peer = get_enet_peer(peer_to_disconnect);
        enet_uint32 data = 0;
        enet_peer_disconnect(peer, data);
}

    mrb_value DRPeer::GetNextEvent(mrb_state *state){
        if (!m_host) {
            return print::print(state, print::PRINT_ERROR, "Tried to index a nil host!");
        }
        ENetEvent event;
        int timeout = 0, out;

        out = enet_host_service(m_host, &event, timeout);

        while(out > 0){
            // filter events

            if(event.channelID < DEFAULT_CHANNEL){

            }

            // check for special events
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    if(!m_is_host){
                        m_is_connected = true;
                    }
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    if(!m_is_host){
                        m_is_connected = false;
                    }
                    break;
                default:
                    break;
            }

            // create simplified copy of event
            auto peer_key = get_peer_key(event.peer);
            auto copy = (simplified_enet_event_t*)MALLOC(sizeof(simplified_enet_event_t));
            copy->peer_key = peer_key;
            copy->type = event.type;
            copy->channel_id = event.channelID;

            auto copied_data = (enet_uint8 *)MALLOC(event.packet->dataLength);
            memcpy(copied_data, event.packet->data, event.packet->dataLength);

            copy->data = copied_data;
            copy->data_length = event.packet->dataLength;

            enet_packet_destroy(event.packet);

            filtered_events.push(copy);

            out = enet_host_service(m_host, &event, timeout);
        }

        if (filtered_events.empty()){
            return mrb_nil_value();
        }

        /*
        if (out < 0){
            return print::print(state, print::PRINT_ERROR, "Error during service");
        }
         */

        auto front = filtered_events.front();
        auto result = event_to_hash(state, front);
        filtered_events.pop();
        FREE(front->data);
        FREE(front);
        return result;
}

    void DRPeer::Send(mrb_state *state, mrb_value data, mrb_int receiver){
        //auto sym_flag = cext_hash_get_sym_default(state, h, str_flag, socket_order_flag_reliable);
        //auto channel_id = cext_hash_get_int_default(state, h, str_channel, 0);
        auto sym_flag = socket_order_flag_reliable;
        auto channel_id = DEFAULT_CHANNEL;

        auto flag = ENET_PACKET_FLAG_RELIABLE;

        if(sym_flag == socket_order_flag_unreliable){
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }else if(sym_flag == socket_order_flag_unsequenced){
            flag = ENET_PACKET_FLAG_UNSEQUENCED;
        }

        auto buffer = new buffer::BinaryBuffer();
        serialize::serialize_data(buffer, state, data);

        ENetPacket * packet = enet_packet_create (buffer->Data(),
                                                  buffer->Size(),
                                                  flag);

        ENetPeer *peer = get_enet_peer(receiver);

        int ret = enet_peer_send(peer, channel_id, packet);
        if (ret < 0) {
            enet_packet_destroy(packet);
        }

        delete buffer;
}

    bool DRPeer::IsConnected(){
        if(!m_is_host){
            return m_is_connected;
        }else{
            return false;
        }
}

    mrb_value dr_peer_initialize(mrb_state *state, mrb_value self) {
        mrb_int is_host, port, only_local;
        API->mrb_get_args(state, "iii", &is_host, &port, &only_local);
        auto peer = new DRPeer(state, is_host != 0, port, only_local != 0);
        dr_peers[peer_counter] = peer;
        auto to_return = peer_counter;
        peer_counter++;

        dr_peer_struct = { "Peer", [](mrb_state *mrb, void *data) {
            auto dr_peer = (DRPeerStruct*)data;
            auto peer_id = dr_peer->internal_peer_id;
            auto peer = dr_peers[peer_id];
            dr_peers.erase(peer_id);
            delete peer;
            API->mrb_free(mrb, data);
        }};

        DRPeerStruct *p;

        p = (DRPeerStruct*)API->mrb_malloc(state, sizeof(DRPeerStruct));
        p->internal_peer_id = to_return;

        DATA_PTR(self) = p;
        DATA_TYPE(self) = &dr_peer_struct;

        return self;
}
    mrb_int get_peer_id(mrb_state *state, mrb_value self) {
        auto dr_ps = (DRPeerStruct*)API->mrb_data_get_ptr(state, self, &dr_peer_struct);
        if (dr_ps == nullptr) {
            //TODO: add exception or similar
        }
        return dr_ps->internal_peer_id;
    }
}
