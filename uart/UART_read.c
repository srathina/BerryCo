
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#define TIMEDELAY 100

int main ()
{
  int serial_port ;

  char gps;
  int count;
  char nmea_frame[100];
  int token_cnt;

  /*
  ** Gps info strings
  */
  char lat[30];
  char lng[30];
  char Validity[5];
  char ns[5];
  char ew[5];

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
      ** capture nmea frame which is between '$'
      */
      if (gps == '$')
      {
        count = 0;
        gps = serialGetchar(serial_port);
        nmea_frame[count] = gps;
        // putchar(gps);

        while (gps != '$')
        {
            gps = serialGetchar(serial_port);
            nmea_frame[++count] = gps;
            fflush (stdout);
         }

        nmea_frame[count] = '\0';
        // printf("%s", nmea_frame);

        /*
        ** separate the GPGLL frame
        */
        char * token = strtok(nmea_frame, ",");

        if ( strcmp(token,"GPGLL") == 0)
        {
            token_cnt = 0;

            printf("\n");
            // loop through the string to extract all other tokens
            while( token != NULL )
            {
                /*
                ** Extract and print gps info...
                */
                if(token_cnt == 1)
                {
                    strcpy(lat,token);
                    printf("Latitude: %s\n",lat);
                }
                else if(token_cnt == 3)
                {
                    strcpy(lng,token);
                    printf("Longitude: %s\n",lng);
                }
                else if(token_cnt == 6)
                {
                    strcpy(Validity,token);
                    printf("Validity: %s\n",Validity);
                }
                else if(token_cnt == 2)
                {
                    strcpy(ns,token);
                    printf("N/S: %s\n",ns);
                }
                else if(token_cnt == 4)
                {
                    strcpy(ew,token);
                    printf("E/W: %s\n",ew);
                }
                token_cnt++;

                // printf( " %s\n", token ); //printing each token
                token = strtok(NULL, ",");
            }

        }

      }

  }


}

