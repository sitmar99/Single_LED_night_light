CC = avr-gcc
MCU=atmega328p
PORT=$(shell pavr2cmd --prog-port)
TTL=$(shell pavr2cmd --ttl-port)
TARGET = main
CFLAGS=-g -Wall -mmcu=$(MCU) -Ofast

all: compile build program clean

size: compile
	avr-size -C --mcu=$(MCU) $(TARGET).elf

clean:
	rm *.o *.elf *.hex

compile:
	$(CC) $(CFLAGS) -c main.c
	$(CC) $(CFLAGS) -o $(TARGET).elf *.o

build:
	avr-objcopy -O ihex $(TARGET).elf $(TARGET).hex

program:
	avrdude -c stk500v2 -P "$(PORT)" -p $(MCU) -U flash:w:$(TARGET).hex:i

debug:
	putty -load AVR -serial $(TTL) -sercfg 9600,8,1,n,N

assembly:
	$(CC) $(CFLAGS) -S main.c