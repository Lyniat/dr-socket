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

    end
end

$socket = DRSocket
)";

#endif