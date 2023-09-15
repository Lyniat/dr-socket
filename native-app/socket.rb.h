#pragma once
#ifndef DR_SOCKET_SOCKET_RB_H
#define DR_SOCKET_SOCKET_RB_H

static const char * ruby_socket_code = R"(

module DRSocket
    class Server
        def initialize port
            @peer_id = __peer_initialize(1, "localhost:#{port}")
        end

        def next_event
            __get_next_event(@peer_id)
        end

        def send receiver, data
            __send(@peer_id, data, receiver)
        end
    end

    class Client
        def initialize
            @peer_id = __peer_initialize(0, "")
        end

        def connect address, port
            __connect(@peer_id, "#{address}:#{port}")
        end

        def next_event
            __get_next_event(@peer_id)
        end

        def send receiver, data
            __send(@peer_id, data, receiver)
        end
    end
end

module GTK
class Runtime
    old_sdl_tick = instance_method(:__sdl_tick__)

    define_method(:__sdl_tick__) do |args|
        old_sdl_tick.bind(self).(args)

        __free_cycle_memory
    end
  end
end
)";

#endif