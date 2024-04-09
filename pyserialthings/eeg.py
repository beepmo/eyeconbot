import serial
import time
import csv
import matplotlib.pyplot as plt

csv_savename = "eeg.csv"

com = "COM5"
baud = 9600

times = []
signal = []
bucket = 0 # make new plot when bucket fills
plt.figure()

flick = 0

try:
    t0 = time.time()
    tp = t0
    with serial.Serial(com, baud, timeout = 0.1) as x:
        
        # flush
        x.reset_input_buffer()
        x.reset_output_buffer()
        x.flush()
        
        while x.isOpen() == True:
            
            x.write(b'd')
            # time.sleep(0.001)
            data = x.read(2)
            
            if len(data) > 0:
                t = time.time() - t0
                times.append(t)
                
                # read integer ADC12MEM0 from bytes: little-endian, unsigned
                ADC12 = int.from_bytes(data, "little")
                print(ADC12)
                volts = ADC12 / 4095 * 3.3
                signal.append(volts)
                
                bucket += 1
                if bucket == 10:
                    # plt.pause(0.2)# wait for plot to update (INCREASE IF PLOT ISN'T UPDATING PROPERLY)
                    plt.clf()
                    plt.plot(times[max(-len(times),-200):],signal[max(-len(times),-200):],'.')
                    plt.show()
                    bucket = 0

            
            # x.write(b'r')
            # print('on!')
            # time.sleep(5)
            
            # x.write(b'l')
            # print('off!')
            # time.sleep(5)
            

                
except KeyboardInterrupt:
    print("Collection stopped - saving to CSV...")
    startcsv = time.time()
    with open(csv_savename, "w", newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        rows = zip(times, signal)
        for row in rows:
            writer.writerow(row)
    endcsv = time.time()
    print("Data saved to "+csv_savename)
    print("Took "+str(endcsv-startcsv)+" s")
