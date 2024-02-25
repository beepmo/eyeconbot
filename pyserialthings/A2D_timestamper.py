import serial
import time
import csv

filename = '0221_STO_J13'
com = "COM4"
baud = 9600

with open(filename, mode='a') as sensor_file:
    sensor_writer = csv.writer(sensor_file)
    sensor_writer.writerow(["R (V)","Time absolute","Time elapsed (s)"])

start = time.time()

x = serial.Serial(com, baud, timeout = 0.1)

while x.isOpen() == True:
    data = str(x.readline().decode('utf-8')).rstrip()
    if len(data) > 0:
        now = time.time()
        print(data)
        with open(filename, mode='a') as sensor_file:
             sensor_writer = csv.writer(sensor_file)
             sensor_writer.writerow([float(data), str(time.asctime()), 
                                     f'{(now - start):.2f}'])
             
             