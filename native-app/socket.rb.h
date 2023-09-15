#pragma once
#ifndef DR_SOCKET_SOCKET_RB_H
#define DR_SOCKET_SOCKET_RB_H

static const char * ruby_socket_code = R"(

include FFI

module DRSocket
    class Client < Peer
        def initialize
            super(0, "")
        end
    end

    class Server < Peer
        def initialize port
            super(1, "localhost:#{port}")
        end
    end
end

module GTK
class Runtime
    old_sdl_tick = instance_method(:__sdl_tick__)

    define_method(:__sdl_tick__) do |args|
        old_sdl_tick.bind(self).(args)

        #DRSocket.__free_cycle_memory
    end
  end
end
)";

#endif