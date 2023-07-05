#ifdef _WIN32

#include <winsock2.h>

#endif

#include <dragonruby.h>
#include "api.h"
#include "socket.h"
#include "help.h"
#include "socket.rb.h"

void ruby_print(mrb_state *state, char *text) {
    drb_api->mrb_funcall(state, mrb_nil_value(), "puts", 1, drb_api->mrb_str_new_cstr(state, text));
}

DRB_FFI
extern "C" {
DRB_FFI_EXPORT
void drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api) {

    drb_api = api;

    drb_api->mrb_load_string(state, ruby_socket_code);

    struct RClass *FFI = drb_api->mrb_module_get(state, "FFI");
    struct RClass *module = drb_api->mrb_module_get(state, "DRSocket");
    //struct RClass *module = drb_api->mrb_define_module_under(state, FFI, "DRSocket");

    //print debug information first
#ifdef META_PLATFORM
    ruby_print(state, (char *) META_PLATFORM);
#else
#error "Missing -DMETA_PLATFORM"
#endif

#ifdef META_TYPE
    ruby_print(state, (char *) META_TYPE);
#else
#error "Missing -DMETA_TYPE"
#endif

#ifdef META_GIT_HASH
    ruby_print(state, (char *) META_GIT_HASH);
#else
#error "Missing -DMETA_GIT_HASH"
#endif

#ifdef META_TIMESTAMP
    ruby_print(state, (char *) META_TIMESTAMP);
#else
#error "Missing -DMETA_TIMESTAMP
#endif

#ifdef META_COMPILER
    ruby_print(state, (char *) META_COMPILER);
#else
#error "Missing -DMETA_COMPILER
#endif

    drb_api->mrb_define_module_function(state, module, "__socket_initialize", {[](mrb_state *mrb, mrb_value self) {
        socket_init();
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_shutdown", {[](mrb_state *mrb, mrb_value self) {
        socket_shutdown();
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_server_destroy", {[](mrb_state *mrb, mrb_value self) {
        socket_server_destroy();
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_client_destroy", {[](mrb_state *mrb, mrb_value self) {
        socket_client_destroy();
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_server_create", {[](mrb_state *mrb, mrb_value self) {
        mrb_value hash, port, max_clients, num_channels;
        drb_api->mrb_get_args(mrb, "H", &hash);

        port = cext_hash_get(mrb, hash, "port");
        if (!cext_is_int(mrb, port)) {
            fprintf(stderr, "'port' could not be found!\n");
            return mrb_nil_value();
        }
        int i_port = static_cast<int>(cext_to_int(mrb, port));

        max_clients = cext_hash_get(mrb, hash, "max_clients");
        if (!cext_is_int(mrb, max_clients)) {
            fprintf(stderr, "'max_clients' could not be found!\n");
            return mrb_nil_value();
        }
        int i_max_clients = static_cast<int>(cext_to_int(mrb, max_clients));

        int i_num_channels = 1;
        num_channels = cext_hash_get(mrb, hash, "channels");
        if (!cext_is_int(mrb, num_channels)) {
            fprintf(stderr, "'num_channels' could not be found. Using default value '%i'.\n", i_num_channels);
        }

        socket_server_create(i_port, i_max_clients, i_num_channels);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(1));

    drb_api->mrb_define_module_function(state, module, "__socket_client_create", {[](mrb_state *mrb, mrb_value self) {
        mrb_value hash, num_channels;
        drb_api->mrb_get_args(mrb, "H", &hash);

        int i_num_channels = 1;
        num_channels = cext_hash_get(mrb, hash, "channels");
        if (!cext_is_int(mrb, num_channels)) {
            fprintf(stderr, "'num_channels' could not be found. Using default value '%i'.\n", i_num_channels);
        }
        socket_client_create(i_num_channels);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(1));

    drb_api->mrb_define_module_function(state, module, "__socket_client_connect", {[](mrb_state *mrb, mrb_value self) {
        mrb_value hash, address, port;
        drb_api->mrb_get_args(mrb, "H", &hash);

        address = cext_hash_get(mrb, hash, "address");
        if (!cext_is_string(mrb, address)) {
            fprintf(stderr, "'address' could not be found!\n");
            return mrb_nil_value();
        }
        const char *s_address = cext_to_string(mrb, address);

        port = cext_hash_get(mrb, hash, "port");
        if (!cext_is_int(mrb, port)) {
            fprintf(stderr, "'port' could not be found!\n");
            return mrb_nil_value();
        }
        int i_port = static_cast<int>(cext_to_int(mrb, port));

        socket_client_connect(s_address, i_port);
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(1));

    drb_api->mrb_define_module_function(state, module, "__socket_server_update", {[](mrb_state *mrb, mrb_value self) {
        socket_server_update();
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_server_fetch", {[](mrb_state *mrb, mrb_value self) {
        mrb_value array = drb_api->mrb_ary_new(mrb);
        for (auto &data_buffer: socket_server_received_buffer) {
            int position = 0;
            mrb_value data = deserialize_data(mrb, data_buffer.buffer, data_buffer.size, &position);
            drb_api->mrb_ary_push(mrb, array, data);
            free((void *) data_buffer.buffer);
        }
        socket_server_received_buffer.clear();
        return array;
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_client_update", {[](mrb_state *mrb, mrb_value self) {
        socket_client_update();
        return mrb_nil_value();
    }}, MRB_ARGS_REQ(0));

    drb_api->mrb_define_module_function(state, module, "__socket_client_send_to_server",
                                        {[](mrb_state *mrb, mrb_value self) {
                                            mrb_value hash, data;
                                            drb_api->mrb_get_args(mrb, "H", &hash);

                                            data = cext_hash_get(mrb, hash, "data");

                                            serialized_data_t serialized_data = serialize_data(mrb, data);

                                            char *buffer = (char *) malloc(1024 * 1024);
                                            int size = serialize_data_to_buffer(buffer, 1024 * 1024, 0,
                                                                                serialized_data);

                                            socket_client_send_to_server(buffer, size);
                                            free(buffer);

                                            return mrb_nil_value();
                                        }}, MRB_ARGS_REQ(1));
}
}
