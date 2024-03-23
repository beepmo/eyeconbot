import serial
import time
import csv
import matplotlib.pyplot as plt

com = "COM5"
baud = 9600

signal = []

with serial.Serial(com, baud, timeout = 0.1) as x:
    
    # flush
    x.reset_input_buffer()
    x.reset_output_buffer()
    x.flush()
    
    while x.isOpen() == True:
        data = x.read(2)
        if len(data) > 0:
            print(data)
            
            # read integer ADC12MEM0 from bytes: little-endian, unsigned
            ADC12 = int.from_bytes(data, "little")
            print(ADC12)
            volts = ADC12 / 4095 * 3.3
            print(f'ADC12MEM0 is {ADC12}, which gives V = {volts}')
            signal.append(volts)
            
            plt.plot(signal,'.')
            plt.show()