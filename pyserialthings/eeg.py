import serial
import time
import csv
import matplotlib.pyplot as plt
import numpy as np

csv_savename = "eeg.csv"
lefts_csv = "lefts.csv"
rights_csv = "rights.csv"

com = "COM5"
baud = 9600

# store smoothed data
times = []
signal = []

# helpers to smooth data
group = []
TSEP = 0.1
VSEP = 0.01
BASE = 1.632

# sliding window for convolution 
p0, p1, p2 = 0, -1, -1
i0, i1 = 0, 0
WINDOW = 0.6
ACCEPT = 0.03
MOVEL = 0.04
MOVER = 0.04
convs = []
tconvs = []
i0s = []
ti0s = []
i1s = []
ti1s = []

# roboarm
QUIET = 2 # only one activation within quiet time 0.1s
lefts = []
rights = []
rcursor = 0
lcursor = 0

last = 0 # store last activation
REPS = 15
SERVOTIME = 0.08 + 0.0001 # (s). drive for three 20 ms periods, then sleep for 100 microprocessor cycles
TA0CCR1 = 1500 # keep track of TA0CCR1 register
JERK = 10 # change in TA0CCR1
MAX = 1800
MIN = 1200


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
                
                
                
                
                if (len(times) > 0 and t - times[-1] < TSEP) \
                and (len(group) > 0 and (np.abs(volts - group[-1]) < VSEP)) \
                or len(group) == 0:
                    # add to cluster
                    group.append(volts)
                else:
                    # complete old cluster, reset new cluster
                    point = np.mean(group) - BASE
                    # if abs(point) > 0.1:
                    #     point *= 2
                    signal.append(point)
                    group = [volts]
                    times.append(t)

                    
                    # slide convolution
                    if times[p1] - times[p0] < WINDOW:
                        # first window isn't grown up yet, and second window isn't born
                        print(f'growing first window')
                        p1 += 1
                        p2 += 1   
                        
                        
                    elif times[p2] - times[p1] < WINDOW:
                        print('growing second window')
                        
                        # grow second window
                        p2 += 1
                                             
                    else:
                        # mature sliding window
                        print(f'\n mature sliding window')
                        print(f'p0, p1, p2 {p0, p1, p2}')
                        print(f'times[p1] - times[p0] {times[p1] - times[p0]}')
                        print(f'times[p2] - times[p1] {times[p2] - times[p1]}')
                        
                        p0 += 1  
                        p1 += 1
                        p2 += 1
                        
                        i0 = np.trapz(signal[p0:p1],times[p0:p1])
                        i1 = np.trapz(signal[p1:p2],times[p1:p2])
                        
                        # save mature i0, i1
                        i0s.append(i0)
                        ti0s.append(times[p0])
                        i1s.append(i1)
                        ti1s.append(times[p0])
                        
                        print(f'i0 {i0}')
                        print(f'i1 {i1}')
                        
                        
                     # plot completed cluster
                    plt.clf()
                    plt.plot(times[max(-len(signal),-50):],signal[max(-len(signal),-50):],'-',label='data')
                    plt.plot(ti0s[max(-len(i0s),-50):],i0s[max(-len(i0s),-50):],'-',label='i0')
                    plt.plot(ti1s[max(-len(i1s),-50):],i1s[max(-len(ti1s),-50):],'-',label='i1')
                    
                    
                    if times[p0] - last > QUIET and (abs(i1) > ACCEPT or abs(i0) > ACCEPT):
                        # take convolution
                        conv = i0 - i1
                        print(f'\ntimes[p0] - last = {times[p0] - last}\nconvolution taken \n')
                        last = times[p0]
                    else:
                        conv = 0
                    convs.append(conv)
                    tconvs.append(times[p0])
                    plt.plot(tconvs[max(-len(signal),-50):],convs[max(-len(signal),-50):],'.',label='conv')
                    plt.axhline(MOVEL)
                    plt.axhline(-MOVER)
                    plt.legend()
                        
                    if conv > MOVEL:
                        # up down
                        print('left')
                        lefts.append(times[p0])
                        last = times[p0]
                        
                        for i in range(REPS):
                            if TA0CCR1 > MAX: break
                            x.write(b'l')
                            TA0CCR1 += JERK;
                            print(f'TA0CCR1 {TA0CCR1}')
                            time.sleep(SERVOTIME)
                    
                    elif -conv > MOVER:
                        # down up
                        print('right')
                        rights.append(times[p0])
                        last = times[p0]
                        
                        for i in range(REPS):
                            if TA0CCR1 < MIN: break
                            x.write(b'r')
                            TA0CCR1 -= JERK;
                            print(f'TA0CCR1 {TA0CCR1}')
                            time.sleep(SERVOTIME)
                            
                        
                    
                    leftbound = times[max(-len(signal),-50)]
                    for t in rights[rcursor:]:
                        if t < leftbound:
                            rcursor += 1
                        else:
                            plt.axvline(t,color='r')
                            plt.axvline(t + 2*WINDOW,color='k')
                    for t in lefts[lcursor:]:
                        if t < leftbound:
                            lcursor += 1
                        else:
                            plt.axvline(t,color='b')
                            plt.axvline(t + 2*WINDOW,color='k')
                    plt.show()

                
except KeyboardInterrupt:

    with open(csv_savename, "w", newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        rows = zip(times, signal)
        for row in rows:
            writer.writerow(row)
    print("Data saved to "+csv_savename)
    
    with open(lefts_csv, "w", newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        for l in lefts:
            writer.writerow([l])
    
    with open(rights_csv, "w", newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        for r in rights:
            writer.writerow([r])
    
