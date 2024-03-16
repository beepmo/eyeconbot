import serial
import time
import csv
import matplotlib.pyplot as plt

com = "COM5"
baud = 9600

x = serial.Serial(com, baud, timeout = 0.05)

start = time.time()

times=[]
temps=[]

while x.isOpen() == True:
    data = x.read(1)
    if len(data) > 0:
        
        hexdata = data.hex()
        
        # counts at 16384 Hz
        counts = int.from_bytes(data,"big")
        
        # duration between TRIGGER and ECHO (s)
        dur = counts / 16384
        
        # measured dist (cm)
        dist = dur * 34000 / 2
        
        # ultrasoud seems to have internal offset
        dist = dist - 37
        
        print(f'hex = {hexdata}, counts = {counts}, dur = {dur}, dist = {dist}')
        
        t=time.time()-start
        
        plt.scatter(times,temps,c='b')
        times.append(t)
        temps.append(dist)
        plt.pause(0.05)
        plt.show()
       