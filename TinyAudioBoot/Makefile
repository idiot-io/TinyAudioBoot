# Makefile
# X-Y Badge
# author: stahl

PROGRAMMER = avrisp
PROGRAMMERSERIALPORT = /dev/ttyACM0

F_CPU = 16000000

DEVICE = attiny85


BOOTLOADER_ADDRESS = 0E00 #for 1k bootloader


FUSEOPT = -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m

# tools
AVRDUDE = avrdude -b19200 -P $(PROGRAMMERSERIALPORT) -c $(PROGRAMMER) -p $(DEVICE) 
CC = avr-gcc

# options
CFLAGS = -std=c99 -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)
LDFLAGS = -Wl,--section-start=.text=$(BOOTLOADER_ADDRESS)


OBJECTS = TinyAudioBoot.o

# symbolic targets
all: TinyAudioBoot.hex

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ -Wa,-ahls=$<.lst

flash: all
	$(AVRDUDE) -U flash:w:TinyAudioBoot.hex:i

readflash:
	$(AVRDUDE) -U flash:r:read.hex:i

fuse:
	$(AVRDUDE) $(FUSEOPT)


clean:
	rm -f TinyAudioBoot.hex TinyAudioBoot.bin *.o TinyAudioBoot.c.lst TinyAudioBoot.map

# file targets
TinyAudioBoot.bin:	$(OBJECTS)
	$(CC) $(CFLAGS) -o TinyAudioBoot.bin $(OBJECTS) $(LDFLAGS)

TinyAudioBoot.hex:	TinyAudioBoot.bin
	rm -f TinyAudioBoot.hex TinyAudioBoot.eep.hex
	avr-objcopy -j .text -j .data -O ihex TinyAudioBoot.bin TinyAudioBoot.hex
	avr-size TinyAudioBoot.hex

disasm: TinyAudioBoot.bin
	avr-objdump -d TinyAudioBoot.bin

cpp:
	$(CC) $(CFLAGS) -E TinyAudioBoot.c

