/*
   FILE: gps1_5.c
   AUTH: P.OH
   DATE: 05/04/02 17:30
   STAT: 05/04/02 15:40 1.0 - Started - works.
   05/04/02 16:00 1.1 - Started. Try to add line breaks - works
   05/04/02 17:30 1.2 - Started. Read/Store chars in string
   05/10/02 07:00 1.3 - Started. 1.2 crashing badly
      19:04 1.3 - Works!  NB: Adding null terminator to gpsString worked
   05/10/02 19:30 1.4 - Started - Extract key info from GPS strings read e.g. time of day - works!
   05/19/02 19:30 1.5 - Clean up 1.4 with nicer text formatting

   DESC: Garmin EMap connected to COM1
   REFS: Uses ibmcom serial libraries
   NOTE: To compile: tcc -ml scon1_1.c ibmcom3.obj
   TODO: longitude cardinal.  Convert into functions
*/

/* Defines required for serial i/o */
#define COM_PORT   1    /* Serial device connected to COM 1 */
#define SPEED      4800   /* baud rate = 4800 */
#define CR         0x0d
#define LF         0x0a
#define ESC        0x1b
#define BEEP       0x07

/* Some helpful defines */
#define SPACE   0x20
#define COMMA   0x2C
#define MAXSIZE    100    /* GPS at most, sends 80 or so chars per message string.  So set maximum to 100 */

#include <stdio.h>
#include <ctype.h>      /* required for the isalnum function */
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <dos.h>
#include "ibmcom3.h"      /* for serial */

/* Prototypes */
void comm_setting(void);    /* Set com port */
void close_com(void);       /* Close com port */

int main(void) {

  unsigned char  charRead;          /* char read from COM port */
  unsigned char  stringRead[MAXSIZE];     /* Buffer collects chars read from GPS */
  unsigned char  tempString[MAXSIZE];
  unsigned char  timeString[12];
  unsigned char  latitudeString[11];
  unsigned char  latitudeCardinalString[3];
  unsigned char  longitudeString[12];
  unsigned char  longitudeCardinalString[3];

  unsigned char  *pChar;
  unsigned char  dummyChar;

  unsigned  long utcTime, estTime;    /* Coordinated Universal Time and Eastern Standard Time */
  unsigned  long utcHour, estHour;
  unsigned  long utcMinutes, estMinutes;
  unsigned  long utcSeconds, estSeconds;
  unsigned  char lastCommaPosition;

  float        latitude;
  int        latDegrees;
  float        latMinutes;

  float    longitude;
  int    longDegrees;
  float    longMinutes;

  FILE           *gpsFile;          /* Text file of GPS strings read */
  unsigned int   j, k;        /* dummy variable */
  unsigned int   i;           /* Number of chars read per GPS message string */
  unsigned int   numLinesRead;            /* Number of GPS strings read */

  dummyChar = 'A'; pChar = &dummyChar;
  gpsFile = fopen("gpsData.txt", "w");

  printf("Initializing port...");
  comm_setting();
  printf("done\n");

  numLinesRead = 0;

  printf("Entering while loop...\n");
  do {
      charRead = com_rx();    /* read char from serial port */
      if(charRead == '$') {     /* GPS messages start with $ char */
    i = 0;
    numLinesRead++;
    stringRead[i] = charRead;
    do {
       charRead = com_rx();
       if( (charRead != '\0') && (isalnum(charRead) ||  isspace(charRead) || ispunct(charRead)) ) {
    i++;
    stringRead[i] = charRead;
       }
    } while(charRead != CR);

    /* By this point, a complete GPS string has been read so save it to file */
    /* Append the null terminator to the string read */
    stringRead[i+1] = '\0';

    /* Analyze string that we collected */
    j = 0;
    pChar = stringRead;
    while(*(pChar+j) != COMMA) {
         tempString[j] = *(pChar+j);
         j++;
    }
    tempString[j] = '\0';

    /* Check if string we collected is the $GPGGA message */
    if(tempString[3] == 'G' && tempString[4] == 'G' && tempString[5] == 'A') {
        /*
     Found GPGGA string.  It has 14 commas total.  Its NMEA sentence structure is:

     $GPGAA,hhmmss.ss,ddmm.mmmm,n,dddmm.mmmm,e,q,ss,y.y,a.a,z,g.g,z,t.t,iii*CC<CR><LF>
     |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
     0       1         2         3         4         5         6         7
     0123456789012345678901234567890123456789012345678901234567890123456789012

     where:

     GPGAA    : GPS fixed data identifier
     hhmmss.ss  : Coordinated Universal Time (UTC), also known as GMT
     ddmm.mmmm,n  : Latitude in degrees, minutes and cardinal sign
     dddmm.mmmm,e : Longitude in degrees, minutes and cardinal sign
     q    : Quality of fix.  1 = there is a fix
     ss   : Number of satellites being used
     y.y    : Horizontal dilution of precision
     a.a,M    : GPS antenna altitude in meters
     g.g,M    : geoidal separation in meters
     t.t    : Age of the defferential correction data
     iiii   : Deferential station's ID
     *CC    : checksum for the sentence
        */

        pChar = stringRead;

        /* Get UTC time */
        j = 7;  /* start of time field */
        k = 0;
        while(*(pChar+j) != COMMA) {
       timeString[k] = *(pChar+j);
       j++;
       k++;
        }
        lastCommaPosition = j;
        timeString[k] = '\0';
        sscanf(timeString, "%ld", &utcTime);
        utcHour = (utcTime/10000);   /* extract Hours from long */
        utcMinutes = (utcTime - (utcHour*10000))/100;  /* extract minutes from long */
        utcSeconds = utcTime - (utcHour*10000) - (utcMinutes*100); /* extract seconds from long */

        if(utcHour >= 4 && utcHour <= 23) estHour = utcHour - 4;
    else estHour = utcHour + 20;
        estMinutes = utcMinutes;
        estSeconds = utcSeconds;

        /* NB: %02ld formats long to print 2 chars wide, padding with 0 if necessary */
        printf("%02ld:%02ld:%02ld UTC = %02ld:%02ld:%02ld EST", utcHour, utcMinutes, utcSeconds, estHour, estMinutes, estSeconds);

        /* Get lattitude: ddmm.mmmm */
        pChar = stringRead;
        j = lastCommaPosition + 1;
        k = 0;
        while(*(pChar+j) != COMMA) {
       latitudeString[k] = *(pChar+j);
       j++;
       k++;
        }
        lastCommaPosition = j;
        latitudeString[k] = '\0';

        sscanf(latitudeString, "%f", &latitude);
        latDegrees = (int)(latitude/100);
        latMinutes = (float)(latitude - latDegrees*100);
        printf("\t%02d DEG\t%2.4f MIN", latDegrees, latMinutes);

        /* Get lattitude Cardinal direction */
        pChar = stringRead;
        j = lastCommaPosition + 1;
        k = 0;
        while(*(pChar+j) != COMMA) {
       latitudeCardinalString[k] = *(pChar+j);
       j++;
       k++;
        }
        lastCommaPosition = j;
        latitudeCardinalString[k] = '\0';
        printf(" %s", latitudeCardinalString);

        /* Get longitude: dddmm.mmmm */
        pChar = stringRead;
        j = lastCommaPosition + 1;
        k = 0;
        while(*(pChar+j) != COMMA) {
       longitudeString[k] = *(pChar+j);
       j++;
       k++;
        }
        lastCommaPosition = j;
        longitudeString[k] = '\0';

        sscanf(longitudeString, "%f", &longitude);
        longDegrees = (int)(longitude/100);
        longMinutes = (float)(longitude - longDegrees*100);
        printf("\t%03d DEG\t%2.4f MIN", longDegrees, longMinutes);

        printf("\n");
    } /* else not a GPGGA sentence */

    fprintf(gpsFile, "%d: (%d) %s\n", numLinesRead, i, stringRead);

      } /* otherwise not a $ character... so loop back until one arrives */
  } while(!kbhit());

  printf("Exiting...");
  close_com();   /* Finished with serial port so close it */
  fclose(gpsFile);
  printf("done\n");
  return (0);

} /* end of main */

void comm_setting(void) {

  int  dummy;

  dummy = com_install(COM_PORT);
  if(dummy != 0) {
  switch (dummy)  {
    case 1  : printf("Invaid port number\n");
        break;
    case 2  : printf("No UART fot specified port\n");
        break;
    case 3  : printf("Drivers already installed\n");
        break;
    default : printf("Err #%d\n", dummy);
        break;
  }
  exit(1);
  } com_raise_dtr();

  com_set_speed(SPEED);
  com_set_parity(COM_NONE, STOP_BIT_1);
}

void close_com(void) {
  com_lower_dtr();
  com_deinstall();
}
