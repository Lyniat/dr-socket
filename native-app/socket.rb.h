#ifndef DR_SOCKET_SOCKET_RB_H
#define DR_SOCKET_SOCKET_RB_H

static const char * ruby_socket_code = R"(
module DRSocket
    class << self
        def raw
            DRSocket::Raw
        end
    end
end

module DRSocket::Raw
    class << self
        def server
            DRSocket::Raw::Server
        end

        def client
            DRSocket::Raw::Client
        end

        def start
            FFI::DRSocket::__socket_initialize
        end

        def end
            FFI::DRSocket::__socket_shutdown
        end
    end
end

module DRSocket::Raw::Server
    class << self
        def start data
            FFI::DRSocket::__socket_server_create data
        end

        def end
            FFI::DRSocket::__socket_server_destroy
        end

        def update
            FFI::DRSocket::__socket_server_update
        end

        def fetch
            FFI::DRSocket::__socket_server_fetch
        end
    end
end

module DRSocket::Raw::Client
    class << self
        def start data
            FFI::DRSocket::__socket_client_create data
        end

        def end
            FFI::DRSocket::__socket_client_destroy
        end

        def update
            FFI::DRSocket::__socket_client_update
        end

        def connect data
            FFI::DRSocket::__socket_client_connect data
        end

        def send data
            FFI::DRSocket::__socket_client_send_to_server data
        end
    end
end

$socket = DRSocket
)";

#endif