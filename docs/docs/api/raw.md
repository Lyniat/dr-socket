# $socket.raw

see [main.rb](https://github.com/Lyniat/dr-socket/blob/main/app/main.rb) for a working example including most of the following features

```ruby
$socket.raw.host_create({address: "pass.ip.address.here:port"})
```

```ruby
$socket.raw.host_connect({address: "pass.ip.address.here:port"})
```

```ruby
$socket.raw.host_service
```

```ruby
next_event = $socket.raw.host_service
```

```ruby
$socket.raw.peer_send(peer_to_send_to, {data: {message: "hello peer"}})
```

```ruby
$socket.raw.peer_disconnect(peer_to_disconnect)
```