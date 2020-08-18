# dragino lora testing
# Single lora testing app

CC=g++
CFLAGS=-c -Wall
LIBS=-lwiringPi

all: dragino_gps_app

dragino_gps_app: gps_read.o
	$(CC) gps_read.o  -o dragino_gps_app -lm -lgps

gps_read.o: gps_read.c
	$(CC) $(CFLAGS) gps_read.c

clean:
	rm *.o dragino_gps_app
