#ifdef _WIN32

#include <winsock2.h>

#endif

#include <memory.h>
#include <dragonruby.h>
#include "ios.h"
#include "api.h"
#include "enet.h"
#include "socket.rb.h"

void ruby_print(mrb_state *state, const char *text) {
    drb_api->mrb_funcall(state, mrb_nil_value(), "puts", 1, drb_api->mrb_str_new_cstr(state, text));
}

void ruby_print_error(mrb_state *state, const char *text) {
    // drb_api->mrb_funcall(state, drb_api->mrb_class_path(state, drb_console), "log_error", 1, drb_api->mrb_str_new_cstr(state, text));
    drb_api->mrb_funcall(state, mrb_nil_value(), "raise", 1, drb_api->mrb_str_new_cstr(state, text));
}

DRB_FFI
extern "C" {
DRB_FFI_EXPORT
void drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api) {

    auto mem = MALLOC(1024);

    check_allocated_memory();

    FREE(mem);

    check_allocated_memory();

    drb_api = api;

    drb_gtk = drb_api->mrb_module_get(state, "GTK");
    drb_runtime = drb_api->mrb_class_get_under(state, drb_gtk, "Runtime");
    drb_console = drb_api->mrb_class_get_under(state, drb_gtk, "Console");

    drb_api->mrb_load_string(state, ruby_socket_code);

#if !defined(META_PLATFORM)     || \
!defined(META_TYPE)             || \
!defined(META_GIT_HASH)         || \
!defined(META_GIT_BRANCH)       || \
!defined(META_TIMESTAMP)        || \
!defined(META_COMPILER_ID)      || \
!defined(META_COMPILER_VERSION)
#error "Missing some Meta information. See CMakeLists.txt for more information.\n"
#endif

    register_socket_symbols(state);
    socket_open_enet(state);

    /*

    drb_api->mrb_define_module_function(state, module, "__socket_initialize", {[](mrb_state *mrb, mrb_value self) {
        socket_init(mrb);
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
            FREE((void *) data_buffer.buffer);
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

                                            char *buffer = (char *) MALLOC(1024 * 1024);
                                            int size = serialize_data_to_buffer(buffer, 1024 * 1024, 0,
                                                                                serialized_data);

                                            socket_client_send_to_server(buffer, size);
                                            FREE(buffer);

                                            return mrb_nil_value();
                                        }}, MRB_ARGS_REQ(1));
                                        */
}
}
