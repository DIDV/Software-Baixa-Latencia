#simplest ruby program to read from arduino serial,
#using the SerialPort gem
#(http://rubygems.org/gems/serialport)

require "serialport"

# params for serial port
# usaly in the form of /dev/tty.usbmodem*
port_str = "/dev/tty.usbmodem00000001"  #may be different for you
baud_rate = 57600
data_bits = 8
stop_bits = 1
parity = SerialPort::NONE

sp = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)

3.times do
  sp.putc 0b10001111
  puts "0b#{sp.getc.bytes.first.to_s(16)}"
end

sp.close
