# run test: ./dragonruby dr-socket --eval app/tests/test.rb --no-tick

require "app/tests/serialize.rb"

puts $socket.get_build_info
puts "running tests"
$gtk.tests.start