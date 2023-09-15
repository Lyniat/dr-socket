require "app/tests/serialize.rb"

$gtk.ffi_misc.gtk_dlopen("socket")
include FFI::DRSocket

def update_server args
    event = $server.next_event
    while event
        puts event
        if event.type == :s_event_receive
          puts("Got message: #{event.data}.")
          $server.send(event.peer, {data: {message: "pong"}})
        elsif event.type == :s_event_connect
          puts("connected.")
        elsif event.type == :s_event_disconnect
          puts("disconnected.")
        end

        event = $server.next_event
    end
end

def update_client args
    event = $client.next_event
    while event
        puts event.type
        if event.type == :s_event_receive
          puts("Got message: #{event.data}.")
          $client.send(event.peer, {data: {message: "ping"}})
        elsif event.type == :s_event_connect
          puts("connected.")
          $client.send(event.peer, {data: {message: "ping"}})
        elsif event.type == :s_event_disconnect
          puts("disconnected.")
        end

        event = $client.next_event
    end
end

def tick args
    # check_allocated_memory
    if args.state.tick_count == 0
        puts get_build_info
        if $gtk.argv.include?("server")
            puts "Starting application as server."
            args.state.socket_type = :server
            $server = Server.new(6789)
        else
            puts "Starting application as client."
            args.state.socket_type = :client
            $client = Client.new()
            $client.connect("localhost", 6789)
        end
    end

    if args.state.socket_type == :server
        update_server args
    else
        update_client args
    end
end

