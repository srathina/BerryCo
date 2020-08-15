
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <wiringSerial.h>

unsigned int get_temp_string(const unsigned char* , unsigned char* , unsigned int , char );

int main ()
{
  int serial_port ;

  unsigned char gps;
  unsigned int count;
  unsigned char nmea_frame[100];
  unsigned int comma_start;
  unsigned char gps_header[15];
  unsigned char lat[15];
  unsigned char ns[5];
  unsigned char lng[15];
  unsigned char ew[5];
  unsigned char utctime[15];
  unsigned char datavalid[5];
  unsigned char positionmode[5];

  system("sudo chmod 777 /dev/ttyS0");

  if ((serial_port = serialOpen ("/dev/ttyS0", 9600)) < 0)	/* open serial port */
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  if (wiringPiSetup () == -1)					/* initializes wiringPi setup */
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }

  for (;;)
  {

      gps = serialGetchar(serial_port);
      /*
      ** capture nmea frame which is between '$' and <CR><LF>
      */
      if (gps == '$')
      {
        count = 0;
        while (gps != '*') // <End character of data field>
        {
            gps = serialGetchar(serial_port);
            nmea_frame[count++] = gps;
         }

        nmea_frame[count] = '\0';

        /*
        ** extract gps frame GPGLL
        */
        comma_start = get_temp_string(nmea_frame, gps_header, 0,',');
        if (strcmp((char *)gps_header,"GPGLL") == 0)
        {
          comma_start = 5;
          comma_start = get_temp_string(nmea_frame, lat, comma_start+1,',');
          comma_start = get_temp_string(nmea_frame, ns, comma_start+1,',');
          comma_start = get_temp_string(nmea_frame, lng, comma_start+1,',');
          comma_start = get_temp_string(nmea_frame, ew, comma_start+1,',');
          comma_start = get_temp_string(nmea_frame, utctime, comma_start+1,',');
          comma_start = get_temp_string(nmea_frame, datavalid, comma_start+1,',');
          comma_start = get_temp_string(nmea_frame, positionmode, comma_start+1,'*');
          printf("%s\n",gps_header);
          printf("%s\n",lat);
          printf("%s\n",ns);
          printf("%s\n",lng);
          printf("%s\n",ew);
          printf("%s\n",utctime);
          printf("%s\n",datavalid);
          printf("%s\n",positionmode);
          printf("\n");
        }

      }

  }

}

unsigned int get_temp_string(const unsigned char* src_str, unsigned char* dest_str, unsigned int start, char delimit)
{

        unsigned int j = 0;

        while (src_str[start] != delimit)
        {
            dest_str[j] = src_str[start];
            start++;
            j++;
        }
        dest_str[j] = '\0';
        return(start);

}
