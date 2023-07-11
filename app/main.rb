class IOSWizard < Wizard
  def before_create_payload
    puts "* INFO - before_create_payload"
    puts "#{app_path}"

    # stage extension
    sh "cp ./dr-socket/native/ios-device/Info.plist ./dr-socket/native/ios-device/socket.framework/Info.plist"
    sh "mkdir -p \"#{app_path}/Frameworks/socket.framework/\""
    sh "cp -r \"#{root_folder}/native/ios-device/socket.framework/\" \"#{app_path}/Frameworks/socket.framework/\""

    # sign
    sh <<-S
    CODESIGN_ALLOCATE=#{codesign_allocate_path} #{codesign_path} \\
                                                -f -s \"#{certificate_name}\" \\
                                                \"#{app_path}/Frameworks/socket.framework/socket\"
    S
   end
end

def tick args
    if args.state.tick_count == 0
        if !args.gtk.platform?(:ios)
            $gtk.ffi_misc.gtk_dlopen("socket")
            include FFI::DRSocket
            $gtk.console.show
        end
    end
  if args.state.tick_count == 180
    if args.gtk.platform?(:ios)
      args.gtk.dlopen "socket"
      include FFI::DRSocket
      $gtk.console.show
      $socket.raw.start
      $socket.raw.server.start({
                      port: 1234,
                      max_clients: 32,
                  })
    end
    return
    argv = args.gtk.argv
    if argv.include?("server")
        puts "Starting application as server."
        args.state.socket_type = :server
    else
        puts "Starting application as client."
        args.state.socket_type = :client
    end

    $socket.raw.start

    if args.state.socket_type == :server
            $socket.raw.server.start({
                port: 1234,
                max_clients: 32,
            })
    end

    if args.state.socket_type == :client
        $socket.raw.client.start({})
        $socket.raw.client.connect({
            address: "127.0.0.1",
            port: 1234,
        })
    end

  end

  return

  if args.state.socket_type == :server
    $socket.raw.server.update
    data = $socket.raw.server.fetch
    if data.size > 0
        puts data
        data.each do |packet|
            if packet.key?(:x)
                args.state.position.x = packet.x
            end
            if packet.key?(:y)
                args.state.position.y = packet.y
            end
            if packet.key?(:pressed)
                args.state.pressed = packet.pressed
            end
        end

    end
    if args.gtk.quit_requested?
        $socket.raw.server.end
        $socket.raw.end
    end
    args.outputs.labels << {
                                 x:                         0,
                                 y:                         0,
                                 text:                      "server",
                                 size_enum:                 2,
                                 r:                         155,
                                 g:                         50,
                                 b:                         50,
                                 a:                         255,
                                 vertical_alignment_enum:   0,
                             }
  end

  if args.state.socket_type == :client
    if args.inputs.keyboard.right
        $socket.raw.client.send(
        {
            data: {
                text: "Client pressed right!",
                number: 764,
                sym: :my_symbol,
                more_data: {
                    s_0: "first string",
                    position: 9,
                    s_1: "second string",
                    collection: [
                        "A",
                        "B",
                        {
                           color_0: "RED",
                           color_1: "GREEN",
                           pi: 3.14
                        },
                    ],
                    game_over: false,
                    has_player: true,
                },
            }
        }
        )
    end
    $socket.raw.client.send({
        data: {
            x: args.inputs.mouse.x,
            y: args.inputs.mouse.y,
            pressed: args.inputs.mouse.button_left,
            }
        }
        )
    args.state.position.x = args.inputs.mouse.x
    args.state.position.y = args.inputs.mouse.y
    args.state.pressed = args.inputs.mouse.button_left
    $socket.raw.client.update
    if args.gtk.quit_requested?
        $socket.raw.client.end
        $socket.raw.end
    end
    args.outputs.labels << {
                                 x:                         0,
                                 y:                         0,
                                 text:                      "client",
                                 size_enum:                 2,
                                 r:                         155,
                                 g:                         50,
                                 b:                         50,
                                 a:                         255,
                                 vertical_alignment_enum:   0,
                             }
  end

  args.outputs.solids << {
      x:    args.state.position.x,
      y:    args.state.position.y,
      w:  100,
      h:  100,
      r:  args.state.pressed ? 255 : 0,
      g:  255,
      b:    0,
      a:  255
    }

  args.outputs.primitives << args.gtk.current_framerate_primitives

  # puts args.gtk.quit_requested?
end
