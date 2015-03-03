#simplest ruby program to read from arduino serial,
#using the SerialPort gem
#(http://rubygems.org/gems/serialport)

require "serialport"
require "pry"

# params for serial port
# usaly in the form of /dev/tty.usbmodem*
port_str = "/dev/tty.usbmodem00000001"  #may be different for you
baud_rate = 57600
data_bits = 8
stop_bits = 1
parity = SerialPort::NONE

sp = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)

def send_data sp, data
  puts "Sending '#{data}'..."
  data.each_char { |c| sp.putc c }
  puts "Data sent."
end

def get_data sp, times=nil
  # this first version just gets data forever
  puts "Getting getting data..."
  if (times)
    times.times { puts sp.getc }
  else
    loop { puts sp.getc }
  end
end

def send_and_get_data sp, data
  send_data sp, data
  get_data sp, data.size
end

binding.pry

sp.close
