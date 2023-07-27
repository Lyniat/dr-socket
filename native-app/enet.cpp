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
#include "enet.h"
#include "ext.h"
#include "api.h"

#include "lua.h"
#include "help.h"
#include "serialize.h"

/*
#define check_host(l, idx)\
	*(ENetHost**)luaL_checkudata(l, idx, "enet_host")

#define check_peer(l, idx)\
	*(ENetPeer**)luaL_checkudata(l, idx, "enet_peer")
*/

using namespace lyniat::socket;
namespace lyniat::socket::enet {

#define check_host(l, idx)\
	get_enet_host()

#define check_peer(l, idx)\
	get_enet_peer(idx)

#define define_function(f, args) \
    API->mrb_define_module_function(state, module, #f, f, MRB_ARGS_REQ(args))

#define undefine_function(f, args) \
    API->mrb_define_module_function(state, module, #f, {[](mrb_state *mrb, mrb_value self) { \
        luaL_error(mrb, "Function %s, is currently not implemented. Sorry :(", #f); \
        return mrb_nil_value(); \
    }}, MRB_ARGS_REQ(args))

#define ENET_ALIGNOF(x) alignof(x)

ENetHost* socket_enet_host;
ENetPeer* socket_enet_peer;

std::map<uintptr_t, ENetPeer *> socket_enet_peers;
std::vector<socket_event_t> socket_enet_events;

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

ENetHost* get_enet_host(){
    return socket_enet_host;
}

ENetPeer* get_enet_peer(uintptr_t id){
    return socket_enet_peers[id];
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
}

/**
 * Parse address string, eg:
 *	*:5959
 *	127.0.0.1:*
 *	website.com:8080
 */
void parse_address(lua_State *l, const char *addr_str, ENetAddress *address) {
    int host_i = 0, port_i = 0;
    char host_str[128] = {0};
    char port_str[32] = {0};
    int scanning_port = 0;

    char *c = (char *)addr_str;

    while (*c != 0) {
        if (host_i >= 128 || port_i >= 32 ) luaL_error(l, "Hostname too long");
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

    if (host_i == 0) luaL_error(l, "Failed to parse address");
    if (port_i == 0) luaL_error(l, "Missing port in address");

    if (strcmp("*", host_str) == 0) {
        address->host = ENET_HOST_ANY;
    } else {
        if (enet_address_set_host(address, host_str) != 0) {
            luaL_error(l, "Failed to resolve host name");
        }
    }

    if (strcmp("*", port_str) == 0) {
        address->port = ENET_PORT_ANY;
    } else {
        address->port = atoi(port_str);
    }
}

/**
 * Find the index of a given peer for which we only have the pointer.
 */
size_t find_peer_index(lua_State *l, ENetHost *enet_host, ENetPeer *peer) {
    size_t peer_index;
    for (peer_index = 0; peer_index < enet_host->peerCount; peer_index++) {
        if (peer == &(enet_host->peers[peer_index]))
            return peer_index;
    }

    luaL_error (l, "enet: could not find peer id!");

    return peer_index;
}

uintptr_t compute_peer_key(lua_State *L, ENetPeer *peer)
{
    // ENet peers are be allocated on the heap in an array. Lua numbers
    // (doubles) can store all possible integers up to 2^53. We can store
    // pointers that use more than 53 bits if their alignment is guaranteed to
    // be more than 1. For example an alignment requirement of 8 means we can
    // shift the pointer's bits by 3.

    // Please see these for the reason of this ternary operator:
    // * https://github.com/love2d/love/issues/1916
    // * https://github.com/love2d/love/commit/4ab9a1ce8c
    const size_t minalign = sizeof(void*) == 8 ? std::min(ENET_ALIGNOF(ENetPeer), ENET_ALIGNOF(std::max_align_t)) : 1;
    uintptr_t key = (uintptr_t) peer;

    if ((key & (minalign - 1)) != 0)
    {
        luaL_error(L, "Cannot push enet peer to Lua: unexpected alignment "
                      "(pointer is %p but alignment should be %d)", peer, minalign);
    }

    static const size_t shift = (size_t) log2((double) minalign);

    return key >> shift;
}

/*
void push_peer_key(lua_State *L, uintptr_t key)
{
    // If full 64-bit lightuserdata is supported (or it's 32-bit platform),
    // always use that. Otherwise, if the key is smaller than 2^53 (which is
    // integer precision for double datatype) on 64-bit platform, then push
    // number. Otherwise, throw error.
    if (supports_full_lightuserdata(L))
        lua_pushlightuserdata(L, (void*) key);
#if UINTPTR_MAX == 0xffffffffffffffff
    else if (key > 0x20000000000000ULL) // 2^53
        luaL_error(L, "Cannot push enet peer to Lua: pointer value %p is too large", key);
#endif
    else
        lua_pushnumber(L, (lua_Number) key);
}
*/

uintptr_t push_peer(lua_State *l, ENetPeer *peer) {
    uintptr_t key = compute_peer_key(l, peer);

    // try to find in peer table
    // lua_getfield(l, LUA_REGISTRYINDEX, "enet_peers");
    // push_peer_key(l, key);
    // lua_gettable(l, -2);
    // auto found_peer = socket_enet_peers.at(key);


    if(!socket_enet_peers.count(key)){
        socket_enet_peers[key] = peer;
    }

    return key;
}

mrb_value push_event(lua_State *l, ENetEvent *event) {
    //lua_newtable(l); // event table

    auto hash = API->mrb_hash_new(l);

    ENetPeer *peer = nullptr;
    int position;
    mrb_value data;


    if (event->peer) {
        peer = event->peer;
        uintptr_t key = push_peer(l, peer);
        cext_hash_set(l, hash, "peer", API->mrb_int_value(l, (mrb_int)key));
    }

    switch (event->type) {
        case ENET_EVENT_TYPE_CONNECT:
            //lua_pushinteger(l, event->data);
            //lua_setfield(l, -2, "data");

            //lua_pushstring(l, "connect");
            cext_hash_set(l, hash, "type", socket_event_connect);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            //lua_pushinteger(l, event->data);
            //lua_setfield(l, -2, "data");

            //lua_pushstring(l, "disconnect");
            cext_hash_set(l, hash, "type", socket_event_disconnect);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            //lua_pushlstring(l, (const char *)event->packet->data, event->packet->dataLength);
            //lua_setfield(l, -2, "data");

            //lua_pushinteger(l, event->channelID);
            //lua_setfield(l, -2, "channel");

            //lua_pushstring(l, "receive");

            cext_hash_set(l, hash, "type", socket_event_receive);
            cext_hash_set(l, hash, "channel", API->mrb_int_value(l, event->channelID));

            //const char* buffer = (const char*)MALLOC(event->packet->dataLength);
            //memcpy((void*)buffer, event->packet->data, event->packet->dataLength);

            position = 0;
            //mrb_value data = deserialize_data(l, buffer, event->packet->dataLength, &position);
            data = serialize::deserialize_data(l, (const char*)event->packet->data, event->packet->dataLength, &position);

            cext_hash_set(l, hash, "data", data);

            enet_packet_destroy(event->packet);
            break;
        case ENET_EVENT_TYPE_NONE:
            cext_hash_set(l, hash, "type", socket_event_none);
            break;
    }

    socket_event_t socket_event = {.peer = peer, .data = hash};
    socket_enet_events.push_back(socket_event);
    return hash;
}

/**
 * Read a packet off the stack as a string
 * idx is position of string
 */
ENetPacket *read_packet(lua_State *l, int idx, enet_uint8 *channel_id) {
    // TODO: add
}

/**
 * Create a new host
 * Args:
 *	address (nil for client)
 *	[peer_count = 64]
 *	[channel_count = 1]
 *	[in_bandwidth = 0]
 *	[out_bandwidth = 0]
 */
mrb_value host_create(lua_State *l, mrb_value self) {
    ENetHost *host;
    size_t peer_count = 64, channel_count = 1;
    enet_uint32 in_bandwidth = 0, out_bandwidth = 0;

    bool have_address = false;
    ENetAddress address;

    mrb_value h;
        API->mrb_get_args(l, "H", &h);

    auto rb_address = cext_hash_get(l, h, "address");
    const char* str_address = "";

    if(cext_is_string(l, rb_address)){
        have_address = true;
        str_address = cext_to_string(l, rb_address);
        parse_address(l, str_address, &address);
    }

    peer_count = cext_hash_get_int_default(l, h, "peer_count", (mrb_int)peer_count);
    channel_count = cext_hash_get_int_default(l, h, "channel_count", (mrb_int)channel_count);
    in_bandwidth = cext_hash_get_int_default(l, h, "in_bandwidth", in_bandwidth);
    out_bandwidth = cext_hash_get_int_default(l, h, "out_bandwidth", out_bandwidth);

    // printf("host create, peers=%d, channels=%d, in=%d, out=%d\n",
    //		peer_count, channel_count, in_bandwidth, out_bandwidth);
    host = enet_host_create(have_address? &address : nullptr, peer_count,
                            channel_count, in_bandwidth, out_bandwidth);

    if (host == nullptr) {
        return luaL_error(l, "enet: failed to create host (already listening?)");
        //return 2;
    }

    // TODO: add this
    //*(ENetHost**)lua_newuserdata(l, sizeof(void*)) = host;
    //luaL_getmetatable(l, "enet_host");
    //lua_setmetatable(l, -2);

    socket_enet_host = host;

    //return CEXT_INT(l, 1);
    return mrb_nil_value();
}

mrb_value linked_version(lua_State *l, mrb_value self) {
    /*
    lua_pushfstring(l, "%d.%d.%d",
                    ENET_VERSION_GET_MAJOR(enet_linked_version()),
                    ENET_VERSION_GET_MINOR(enet_linked_version()),
                    ENET_VERSION_GET_PATCH(enet_linked_version()));
                    */
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

    auto result = API->mrb_str_new(l, buffer, size);
    FREE(buffer);
    return result;
}

/**
 * Serice a host
 * Args:
 *	timeout
 *
 * Return
 *	nil on no event
 *	an event table on event
 */
mrb_value host_service(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    ENetEvent event;
    int timeout = 0, out;

        API->mrb_get_args(l, "i", &timeout);

    out = enet_host_service(host, &event, timeout);
    if (out == 0){
        return mrb_nil_value();
    }
    if (out < 0){
        return luaL_error(l, "Error during service");
    }

    return push_event(l, &event);
}

/**
 * Dispatch a single event if available
 */
mrb_value host_check_events(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    ENetEvent event;
    int out = enet_host_check_events(host, &event);
    if (out == 0) return mrb_nil_value();
    if (out < 0) return luaL_error(l, "Error checking event");

    push_event(l, &event);
    //return CEXT_INT(l, 1);
    return mrb_nil_value();
}

/**
 * Enables an adaptive order-2 PPM range coder for the transmitted data of
 * all peers.
 */
 // TODO: do wee need this?
 /*
mrb_value host_compress_with_range_coder(lua_State *l) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    int result = enet_host_compress_with_range_coder (host);
    if (result == 0) {
        lua_pushboolean (l, 1);
    } else {
        lua_pushboolean (l, 0);
    }

    return CEXT_INT(l, 1);
}
*/

/**
 * Connect a host to an address
 * Args:
 *	the address
 *	[channel_count = 1]
 *	[data = 0]
 */
mrb_value host_connect(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    ENetAddress address;
    ENetPeer *peer;

    enet_uint32 data = 0;
    size_t channel_count = 1;

    mrb_value h;
        API->mrb_get_args(l, "H", &h);

    auto str_address = cext_hash_get_string_default(l, h, "address", "");

    parse_address(l, str_address, &address);

    channel_count = cext_hash_get_int_default(l, h, "channel_count", (mrb_int)channel_count);
    data = cext_hash_get_int_default(l, h, "data", (mrb_int)data);

    // printf("host connect, channels=%d, data=%d\n", channel_count, data);
    peer = enet_host_connect(host, &address, channel_count, data);

    if (peer == nullptr) {
        return luaL_error(l, "Failed to create peer");
    }

    push_peer(l, peer);

    //return CEXT_INT(l, 1);
    return mrb_nil_value();
}

mrb_value host_flush(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    enet_host_flush(host);
    return mrb_nil_value();
}

mrb_value host_broadcast(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    enet_uint8 channel_id;
    ENetPacket *packet = read_packet(l, 2, &channel_id);
    enet_host_broadcast(host, channel_id, packet);
    return mrb_nil_value();
}

// Args: limit:number
mrb_value host_channel_limit(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    mrb_int limit;
        API->mrb_get_args(l, "i", &limit);
    enet_host_channel_limit(host, limit);
    return mrb_nil_value();
}

// https://leafo.net/lua-enet/#hostbandwidth_limitincoming_outgoing
mrb_value host_bandwidth_limit(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    enet_uint32 in_bandwidth, out_bandwidth;

        API->mrb_get_args(l, "ii", &in_bandwidth, &out_bandwidth);

    enet_host_bandwidth_limit(host, in_bandwidth, out_bandwidth);
    return mrb_nil_value();
}

mrb_value host_get_socket_address(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }
    ENetAddress address;
    enet_socket_get_address (host->socket, &address);

    /*
    lua_pushfstring(l, "%d.%d.%d.%d:%d",
                    ((address.host) & 0xFF),
                    ((address.host >> 8) & 0xFF),
                    ((address.host >> 16) & 0xFF),
                    (address.host >> 24& 0xFF),
                    address.port);
    */

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

    auto result = API->mrb_str_new(l, buffer, size);
    //TODO: free buffer?
    return result;
}

mrb_value host_total_sent_data(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(l, host->totalSentData);
    return result;
}

mrb_value host_total_received_data(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(l, host->totalReceivedData);
    return result;
}

mrb_value host_service_time(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(l, host->serviceTime);
    return result;
}

mrb_value host_peer_count(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    auto result = API->mrb_int_value(l, host->peerCount);
    return result;
}

// https://leafo.net/lua-enet/#hostget_peerindex
// TODO: !
mrb_value host_get_peer(lua_State *l, mrb_value self) {
    ENetHost *host = check_host(l, 1);
    if (!host) {
        return luaL_error(l, "Tried to index a nil host!");
    }

    mrb_int peer_index;
        API->mrb_get_args(l, "i", peer_index);

    if (peer_index < 0 || ((size_t) peer_index) >= host->peerCount) {
        //luaL_argerror (l, 2, "Invalid peer index");
        return luaL_error(l, "Invalid peer index");
    }

    ENetPeer *peer = &(host->peers[peer_index]);

    push_peer (l, peer);
    //return CEXT_INT(l, 1);
    return mrb_nil_value();
}

mrb_value host_gc(lua_State *l) {
    if (socket_enet_host) {
        enet_host_destroy(socket_enet_host);
    }
    socket_enet_host = nullptr;
    return mrb_nil_value();
}

/*
mrb_value peer_tostring(lua_State *l) {
    ENetPeer *peer = check_peer(l, 1);
    char host_str[128];
    enet_address_get_host_ip(&peer->address, host_str, 128);

    lua_pushstring(l, host_str);
    lua_pushstring(l, ":");
    lua_pushinteger(l, peer->address.port);
    lua_concat(l, 3);
    return 1;
}
 */

mrb_value peer_ping(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);
    enet_peer_ping(peer);
    return mrb_nil_value();
}

// https://leafo.net/lua-enet/#peerthrottle_configureinterval_acceleration_deceleration
mrb_value peer_throttle_configure(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    enet_uint32 interval, acceleration, deceleration;
        API->mrb_get_args(l, "iii", &interval, &acceleration, &deceleration);

    enet_peer_throttle_configure(peer, interval, acceleration, deceleration);
    return mrb_nil_value();
}

// TODO: implement this
/*
int peer_round_trip_time(lua_State *l) {
    ENetPeer *peer = check_peer(l, 1);

    if (lua_gettop(l) > 1) {
        enet_uint32 round_trip_time = (int) luaL_checknumber(l, 2);
        peer->roundTripTime = round_trip_time;
    }

    lua_pushinteger (l, peer->roundTripTime);

    return 1;
}
*/

// TODO: implement this
/*
int peer_last_round_trip_time(lua_State *l) {
    ENetPeer *peer = check_peer(l, 1);

    if (lua_gettop(l) > 1) {
        enet_uint32 round_trip_time = (int) luaL_checknumber(l, 2);
        peer->lastRoundTripTime = round_trip_time;
    }
    lua_pushinteger (l, peer->lastRoundTripTime);

    return 1;
}
 */

// https://leafo.net/lua-enet/#peerping_intervalinterval
mrb_value peer_ping_interval(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    enet_uint32 interval;
        API->mrb_get_args(l, "i", &interval);

    enet_peer_ping_interval (peer, interval);

    //return CEXT_INT(l, 1);
    return mrb_nil_value();
}

mrb_value peer_timeout(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    enet_uint32 timeout_limit, timeout_minimum, timeout_maximum;

    API->mrb_get_args(l, "iii", &timeout_limit, &timeout_minimum, &timeout_maximum);

    enet_peer_timeout (peer, timeout_limit, timeout_minimum, timeout_maximum);

    //lua_pushinteger (l, peer->timeoutLimit);
    //lua_pushinteger (l, peer->timeoutMinimum);
    //lua_pushinteger (l, peer->timeoutMaximum);

    auto result = API->mrb_ary_new(l);
    API->mrb_ary_push(l, result, API->mrb_int_value(l, peer->timeoutLimit));
    API->mrb_ary_push(l, result, API->mrb_int_value(l, peer->timeoutMinimum));
    API->mrb_ary_push(l, result, API->mrb_int_value(l, peer->timeoutMaximum));

    return result;
}

//TODO: accept optional value like in https://leafo.net/lua-enet/#peerdisconnectdata
mrb_value peer_disconnect(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    //enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(l, 2) : 0;
    enet_uint32 data = 0;
    enet_peer_disconnect(peer, data);
    return mrb_nil_value();
}

//TODO: accept optional value like in https://leafo.net/lua-enet/#peerdisconnect_nowdata
mrb_value peer_disconnect_now(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    //enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(l, 2) : 0;
    enet_uint32 data = 0;
    enet_peer_disconnect_now(peer, data);
    return mrb_nil_value();
}

//TODO: accept optional value like in https://leafo.net/lua-enet/#peerdisconnect_laterdata
mrb_value peer_disconnect_later(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    //enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(l, 2) : 0;
    enet_uint32 data = 0;
    enet_peer_disconnect_later(peer, data);
    return mrb_nil_value();
}

mrb_value peer_index(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    size_t peer_index = find_peer_index (l, peer->host, peer);

    return API->mrb_int_value(l, (mrb_int)peer_index);
}

mrb_value peer_state(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    switch (peer->state) {
        case (ENET_PEER_STATE_DISCONNECTED):
            //lua_pushstring (l, "disconnected");
            return socket_state_disconnected;
            break;
        case (ENET_PEER_STATE_CONNECTING):
            //lua_pushstring (l, "connecting");
            return socket_state_connecting;
            break;
        case (ENET_PEER_STATE_ACKNOWLEDGING_CONNECT):
            //lua_pushstring (l, "acknowledging_connect");
            return socket_state_acknowledging_connect;
            break;
        case (ENET_PEER_STATE_CONNECTION_PENDING):
            //lua_pushstring (l, "connection_pending");
            return socket_state_connection_pending;
            break;
        case (ENET_PEER_STATE_CONNECTION_SUCCEEDED):
            //lua_pushstring (l, "connection_succeeded");
            return socket_state_connection_succeeded;
            break;
        case (ENET_PEER_STATE_CONNECTED):
            //lua_pushstring (l, "connected");
            return  socket_state_connected;
            break;
        case (ENET_PEER_STATE_DISCONNECT_LATER):
            //lua_pushstring (l, "disconnect_later");
            return  socket_state_disconnect_later;
            break;
        case (ENET_PEER_STATE_DISCONNECTING):
            //lua_pushstring (l, "disconnecting");
            return  socket_state_disconnecting;
            break;
        case (ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT):
            //lua_pushstring (l, "acknowledging_disconnect");
            return  socket_state_acknowledging_disconnect;
            break;
        case (ENET_PEER_STATE_ZOMBIE):
            //lua_pushstring (l, "zombie");
            return socket_state_zombie;
            break;
        default:
            //lua_pushstring (l, "unknown");
            return socket_state_unknown;
    }
}

mrb_value peer_connect_id(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);

    //lua_pushinteger (l, peer->connectID);
    return API->mrb_int_value(l, peer->connectID);
}


mrb_value peer_reset(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);
    enet_peer_reset(peer);

    return mrb_nil_value();
}

//TODO: channel_id?
/*
mrb_value peer_receive(lua_State *l, mrb_value self) {
    ENetPeer *peer = check_peer(l, 1);
    ENetPacket *packet;
    enet_uint8 channel_id = 0;

    if (lua_gettop(l) > 1) {
        channel_id = (int) luaL_checknumber(l, 2);
    }

    packet = enet_peer_receive(peer, &channel_id);
    if (packet == nullptr){
        return mrb_nil_value();
    }

    lua_pushlstring(l, (const char *)packet->data, packet->dataLength);
    lua_pushinteger(l, channel_id);

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
mrb_value peer_send(lua_State *l, mrb_value self) {
    mrb_int peer_id;
    mrb_value h;
    API->mrb_get_args(l, "iH", &peer_id, &h);

    auto data = cext_hash_get_save_hash(l, h, "data");
    auto sym_flag = cext_hash_get_sym_default(l, h, "flag", socket_order_flag_reliable);
    auto channel_id = cext_hash_get_int_default(l, h, "channel", 0);

    auto flag = ENET_PACKET_FLAG_RELIABLE;

    if(sym_flag == socket_order_flag_unreliable){
        flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
    }else if(sym_flag == socket_order_flag_unsequenced){
        flag = ENET_PACKET_FLAG_UNSEQUENCED;
    }

    serialize::serialized_data_t serialized_data = serialize::serialize_data(l, data);

    char *buffer = (char *) MALLOC(1024 * 1024); //TODO: no fixed size!
    int size = serialize_data_to_buffer(buffer, 1024 * 1024, 0,
                                        serialized_data);

    ENetPacket * packet = enet_packet_create (buffer,
                                              size,
                                              flag);

    ENetPeer *peer = check_peer(l, peer_id);

    // printf("sending, channel_id=%d\n", channel_id);
    int ret = enet_peer_send(peer, channel_id, packet);
    if (ret < 0) {
        enet_packet_destroy(packet);
    }

    FREE(buffer);

    return API->mrb_int_value(l, ret);
}

void socket_open_enet(mrb_state* state) {
    struct RClass *FFI = API->mrb_module_get(state, "FFI");
    struct RClass *module_socket = API->mrb_module_get_under(state, FFI, "DRSocket");
    // struct RClass *module = API->mrb_module_get_under(state, module_socket, "Raw");
    struct RClass *module = API->mrb_module_get_under(state, module_socket, "Raw");
    // struct RClass *module = API->mrb_class_get_under(state, module_raw, "Raw");

    //struct RClass *module_socket = API->mrb_define_module_under(state, FFI, "DRSocket");
    //struct RClass *module = API->mrb_define_module_under(state, module_socket, "Raw");

    enet_initialize();
    //atexit(enet_deinitialize); TODO: use this

    //IMPORTANT: to use "define_function" macro, "state" and "module" MUST be defined in this function

    // host
    define_function(host_create, 1);
    define_function(host_connect, 1);
    define_function(host_service, 1);
    define_function(host_check_events, 0);
    undefine_function(enet_host_compress_with_range_coder, 0); //TODO: implement this
    define_function(host_flush, 0);
    define_function(host_broadcast, 0);
    define_function(host_channel_limit, 1);
    define_function(host_bandwidth_limit, 2);
    define_function(host_total_sent_data, 0);
    define_function(host_total_received_data, 0);
    define_function(host_service_time, 0);
    define_function(host_peer_count, 0);
    undefine_function(host_get_peer, 0); //TODO: implement this
    define_function(host_get_socket_address, 0);

    // peer
    define_function(peer_connect_id, 0);
    define_function(peer_disconnect, 0);
    define_function(peer_disconnect_now, 0);
    define_function(peer_disconnect_later, 0);
    define_function(peer_index, 0);
    define_function(peer_ping, 0);
    define_function(peer_ping_interval, 1);
    define_function(peer_reset, 0);
    define_function(peer_send, 2);
    define_function(peer_state, 0);
    undefine_function(peer_receive, 1); //TODO: implement this
    undefine_function(peer_round_trip_time, 1); //TODO: implement this
    undefine_function(last_round_trip_time, 1); //TODO: implement this
    define_function(peer_throttle_configure, 3);
    define_function(peer_timeout, 3);

    API->mrb_define_module_function(state, module_socket, "get_build_info", {[](mrb_state *mrb, mrb_value self) {
        auto enet_version = linked_version(mrb, self);
        auto result = API->mrb_hash_new(mrb);
        cext_hash_set(mrb, result, "enet", enet_version);

        auto meta_platform = (const char *) META_PLATFORM;
        auto meta_type = (const char *) META_TYPE;
        auto meta_git_hash = (const char *) META_GIT_HASH;
        auto meta_git_branch = (const char *) META_GIT_BRANCH;
        auto meta_timestamp = (const char *) META_TIMESTAMP;
        auto meta_compiler_id = (const char *) META_COMPILER_ID;
        auto meta_compiler_version = (const char *) META_COMPILER_VERSION;

        auto build_information = API->mrb_hash_new(mrb);
        cext_hash_set(mrb, build_information, "target_platform", API->mrb_str_new_cstr(mrb, meta_platform));
        cext_hash_set(mrb, build_information, "build_type", API->mrb_str_new_cstr(mrb, meta_type));
        cext_hash_set(mrb, build_information, "git_hash", API->mrb_str_new_cstr(mrb, meta_git_hash));
        cext_hash_set(mrb, build_information, "git_branch", API->mrb_str_new_cstr(mrb, meta_git_branch));
        cext_hash_set(mrb, build_information, "build_time", API->mrb_str_new_cstr(mrb, meta_timestamp));

        auto compiler = API->mrb_hash_new(mrb);
        cext_hash_set(mrb, compiler, "id", API->mrb_str_new_cstr(mrb, meta_compiler_id));
        cext_hash_set(mrb, compiler, "version", API->mrb_str_new_cstr(mrb, meta_compiler_version));

        cext_hash_set(mrb, build_information, "compiler", compiler);

        cext_hash_set(mrb, result, "build_information", build_information);

        return result;
    }}, MRB_ARGS_REQ(0));

    API->mrb_define_module_function(state, module_socket, "check_allocated_memory",
                                        {[](mrb_state *mrb, mrb_value self) {
#ifdef DEBUG
                                            lyniat::memory::check_allocated_memory();
#else
                                            ruby_print(mrb, (char*)"check_allocated_memory is only available in a Debug build.")
#endif
                                            return mrb_nil_value();
                                        }}, MRB_ARGS_REQ(0));

    API->mrb_define_module_function(state, module_socket, "__free_cycle_memory",
                                        {[](mrb_state *mrb, mrb_value self) {
                                            FREE_CYCLE
                                            return mrb_nil_value();
                                        }}, MRB_ARGS_REQ(0));

}
}
