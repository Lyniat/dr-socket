# $socket.raw.client

```ruby
$socket.raw.client.start({
    port: 1234,
    max_clients: 32,
})
```

```ruby
$socket.raw.client.end
```

```ruby
$socket.raw.client.update
```

```ruby
$socket.raw.client.send({
    data: {
        x: args.inputs.mouse.x,
        y: args.inputs.mouse.y,
        pressed: args.inputs.mouse.button_left,
        }
    }
    )
```