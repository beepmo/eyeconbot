import serial
import time
import csv
import matplotlib.pyplot as plt

com = "COM5"
baud = 9600

x = serial.Serial(com, baud, timeout = 0.05)

start = time.time()

while x.isOpen() == True:
    data = x.read(1)
    if len(data) > 0:
        print(data)