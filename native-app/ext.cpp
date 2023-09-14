#ifdef _WIN32

#include <winsock2.h>

#endif

#include "ruby.h"
#include "ios.h"
#include "api.h"
#include "enet.h"
#include "socket.rb.h"

using namespace lyniat::socket;

DRB_FFI
extern "C" {
DRB_FFI_EXPORT
void drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api) {

    API = api;

    api::drb_gtk = API->mrb_module_get(state, "GTK");
    api::drb_runtime = API->mrb_class_get_under(state, api::drb_gtk, "Runtime");
    api::drb_console = API->mrb_class_get_under(state, api::drb_gtk, "Console");

    API->mrb_load_string(state, ruby_socket_code);

#if !defined(META_PLATFORM)     || \
!defined(META_TYPE)             || \
!defined(META_GIT_HASH)         || \
!defined(META_GIT_BRANCH)       || \
!defined(META_COMPILER_ID)      || \
!defined(META_COMPILER_VERSION)
#error "Missing some Meta information. See CMakeLists.txt for more information.\n"
#endif

    enet::register_socket_symbols(state);
    enet::socket_open_enet(state);
}
}
