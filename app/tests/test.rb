# run test: ./dragonruby dr-socket --eval app/tests/test.rb --no-tick

require "app/tests/serialize.rb"

puts "running tests"
$gtk.tests.start