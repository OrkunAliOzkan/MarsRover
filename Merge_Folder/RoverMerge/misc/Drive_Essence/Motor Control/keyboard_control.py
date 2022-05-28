import serial
import keyboard

serialPort = serial.Serial(port="COM5", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
#serialString = ""  # Used to hold data coming over UART

counter = 0
while 1:
    # Wait until there is data waiting in the serial buffer
    counter += 1
    if keyboard.is_pressed('w') and counter % 100 == 0:
        serialPort.write(b'w')
        serialPort.flushOutput()
        # # Read data out of the buffer until a carraige return / new line is found
        # serialString = serialPort.readline()

        # # Print the contents of the serial data
        # try:
        #     print(serialString.decode("Ascii"))
        # except:
        #     pass