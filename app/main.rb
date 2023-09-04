$gtk.ffi_misc.gtk_dlopen("socket")
include FFI::DRSocket

def init_client args
    $socket.raw.host_create({})
    $socket.raw.host_connect({address: "localhost:6789"})
end

def init_server args
    $socket.raw.host_create({address: "localhost:6789"})
end

def update_server args
    event = $socket.raw.host_service
    while event
        puts event
        if event.type == :s_event_receive
          puts("Got message: #{event.data}.")
          $socket.raw.peer_send(event.peer, {data: {message: "pong"}})
        elsif event.type == :s_event_connect
          puts("connected.")
        elsif event.type == :s_event_disconnect
          puts("disconnected.")
        end

        event = $socket.raw.host_service
    end
end

def update_client args
    event = $socket.raw.host_service
    while event
        puts event.type
        if event.type == :s_event_receive
          puts("Got message: #{event.data}.")
          $socket.raw.peer_send(event.peer, {data: {message: "ping"}})
        elsif event.type == :s_event_connect
          puts("connected.")
          $socket.raw.peer_send(event.peer, {data: {message: "ping"}})
        elsif event.type == :s_event_disconnect
          puts("disconnected.")
        end

        event = $socket.raw.host_service
    end
end

def tick args
    if args.state.tick_count == 0
        puts $socket.get_build_info
        if $gtk.argv.include?("server")
            puts "Starting application as server."
            args.state.socket_type = :server
            init_server args
        else
            puts "Starting application as client."
            args.state.socket_type = :client
            init_client args
        end
    end

    if args.state.socket_type == :server
        update_server args
    else
        update_client args
    end
    # $socket.check_allocated_memory
end

