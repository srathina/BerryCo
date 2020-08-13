# dragino lora testing
# Single lora testing app

CC=g++
CFLAGS=-c -Wall
LIBS=-lwiringPi

all: dragino_gps_app

dragino_gps_app: UART_read.o
	$(CC) UART_read.o  $(LIBS) -o dragino_gps_app

UART_read.o: UART_read.c
	$(CC) $(CFLAGS) UART_read.c

clean:
	rm *.o dragino_gps_app
