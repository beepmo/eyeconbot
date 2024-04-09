import serial
import time
import csv
import matplotlib.pyplot as plt
import numpy as np

csv_savename = "eeg.csv"

com = "COM5"
baud = 9600

# store smoothed data
times = []
signal = []

# helper data structure to smooth data
group = []
TSEP = 0.3
VSEP = 0.01
BASE = 1.632

# sliding window for convolution 
p0, p1, p2 = 0, -1, -1
i0, i1 = 0, 0
WINDOW = 0.6
ACCEPT = 0.03
MOVE = 0.06


try:
    t0 = time.time()
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
                
                # read integer ADC12MEM0 from bytes: little-endian, unsigned
                ADC12 = int.from_bytes(data, "little")
                volts = ADC12 / 4095 * 3.3
                print(ADC12)
                print(t)
                
                if (len(times) > 0 and t - times[-1] < TSEP) \
                and (len(group) > 0 and (np.abs(volts - group[-1]) < VSEP)) \
                or len(group) == 0:
                    # add to cluster
                    group.append(volts)
                else:
                    print('new group')
                    # complete old cluster, reset new cluster
                    signal.append(np.mean(group) - BASE)
                    group = [volts]
                    times.append(t)
                
                   
                    
                    print(f'p0, p1, p2 {p0, p1, p2}')
                    print(f'times[p1] - times[p0] {times[p1] - times[p0]}')
                    print(f'times[p2] - times[p1] {times[p2] - times[p1]}')
                    print(f'i0 {i0}')
                    print(f'i1 {i1}')
                    
                    # slide convolution
                    if times[p1] - times[p0] < WINDOW:
                        # first window isn't grown up yet, and second window isn't born
                        print(f'growing first window')
                        p1 += 1
                        p2 += 1
                    
                        # add v*dt to integral i0
                        
                        dt = times[p1] - times[p1 - 1]
                        i0 += signal[p1] * dt
                        
                    elif times[p2] - times[p1] < WINDOW:
                        print('growing second window')
                        
                        # grow second window
                        p2 += 1
                        
                        # add v*dt to integral i1
                        dt = times[p2] - times[p2 - 1]
                        i1 += signal[p2] * dt
                        
                    else:
                        # mature sliding window
                        print('mature sliding window')
                        
                        p0 += 1
                        # update integral i0
                        dt = times[p0] - times[p0 - 1]
                        i0 -= dt * signal[p0 - 1]
                        
                        p1 += 1
                        dt = times[p1] - times[p1 - 1]
                        i0 += signal[p1] * dt
                        
                        i1 -= signal[p1] * dt
                        
                        p2 += 1
                        dt = times[p2] - times[p2 - 1]
                        i1 += signal[p2] * dt
                        
                     # plot completed cluster
                    plt.clf()
                    plt.plot(times[max(-len(signal),-50):],signal[max(-len(signal),-50):],'.')
                    
                    if abs(i1) > ACCEPT and abs(i0) > ACCEPT:
                        # take convolution
                        if i1 - i0 > MOVE:
                            # down up
                            print('right')
                            plt.axvline(times[p0],color='r')
                        elif i0 - i1 > MOVE:
                            # up down
                            print('left')
                            plt.axvline(times[p0],color='b')
                    
                    plt.show()


            
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
