import serial
from csv import writer
import time
import sys
import threading
import itertools

#Animation for 3D fix
def animate():
    for c in itertools.cycle(['|', '/', '-', '\\']):
        if done:
            break
        sys.stdout.write('\rAwaiting 3D fix ' + c)
        sys.stdout.flush()
        time.sleep(0.1)
    sys.stdout.write('\rNo 3D fix! Please troubleshoot the GPS module or Keep the module in open space       ')


# DDMM.MMMM to DMS converter
def decdeg2dms(dd):
    dd = abs(dd)
    degrees = int(int(dd)/100)
    minutes_flt = dd-(degrees*100)
    minutes = int(minutes_flt)
    seconds = (minutes_flt - minutes)*60
    return (degrees,minutes,seconds,minutes_flt)

# csv editor
def append_list_as_row(file_name, list_of_elem):
    with open(file_name, 'a+', newline='') as write_obj:
        csv_writer = writer(write_obj)
        csv_writer.writerow(list_of_elem)
        
# Distance calculator
def dist_calc_lat(d_p,d,m_p,m,s_p,s):
    return ((abs(d_p-d)*110947.2)+(abs(m_p-m)*1849.526)+(abs(s_p-s)*30.7848))
def dist_calc_long(d_p,d,m_p,m,s_p,s):
        return ((abs(d_p-d)*87843.36)+(abs(m_p-m)*1463.04)+(abs(s_p-s)*24.384))
    
        
# UART reading
ser = serial.Serial(
        port='/dev/ttyS0',
        baudrate = 9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
)

# initialising variables for the calculations
lat_dms_prev = (0,0,0,0)
long_dms_prev = (0,0,0,0)
count = 0
done = False
count_fix = 0

while 1:
    x=ser.readline().decode()
    if x.startswith('$GPGLL'):                  # Finding GPGLL NMEA command
        lat_long = x.split(',')                 # Splitting the elements of the GPGLL command
        
        try:
            lat =float(lat_long[1])
            long =float(lat_long[3])
            
            lat_dms = decdeg2dms(lat)
            long_dms = decdeg2dms(long)
            
            dist_lat = dist_calc_lat(lat_dms_prev[0],lat_dms[0],lat_dms_prev[1],lat_dms[1],lat_dms_prev[2],lat_dms[2])
            dist_long = dist_calc_lat(long_dms_prev[0],long_dms[0],long_dms_prev[1],long_dms[1],long_dms_prev[2],long_dms[2])
            if not count==0:
                if dist_lat!=0.0 or dist_long != 0.0:
                    print('Latitudinal distance moved in m:',round(dist_lat,2))
                    print('Longitudinal distance moved in m:',round(dist_long,2),'\n')
            
            # converting to DD.DDDD for Google maps interface                
            lat_gmap = lat_dms[0]+(lat_dms[3]/60)
            long_gmap = long_dms[0]+(long_dms[3]/60)
            
            # UTC from GPGLL NMEA code
            timestamp = lat_long[5]
            UTC_time = timestamp[0:2]+':'+timestamp[2:4]+':'+timestamp[4:6]
            
            print('\nTime(UTC):',UTC_time,'\nLat:',str(lat_dms[0])+'\N{DEGREE SIGN}'+str(lat_dms[1])+'\''+str(round(lat_dms[2],2))+'¨',lat_long[2],'Long:',str(long_dms[0])+'\N{DEGREE SIGN}'+str(long_dms[1])+'\''+str(round(long_dms[2],2))+'¨',lat_long[4])
                    
            if count == 0:                          #Adding row headings fro the first time
                row_contents = ['UTC','Lat','Long']
            else:                                   #Adding further rows for the values 
                row_contents = [UTC_time,lat_gmap,long_gmap]
            append_list_as_row('gps3.csv', row_contents)
            
            count = count+1
            lat_dms_prev = lat_dms
            long_dms_prev = long_dms
        
        except:
            t = threading.Thread(target=animate)
            t.start()
            count_fix +=1
            time.sleep(30)
            if count_fix == 2:
                done = True
                break
