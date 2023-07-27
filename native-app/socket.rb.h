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