# Name: Makefile
# Project: USBaspLoader
# Author: Christian Starkjohann
# Creation Date: 2007-12-10
# Tabsize: 4
# Copyright: (c) 2007 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt)
# This Revision: $Id: Makefile 798 2010-07-27 17:29:28Z cs $


###############################################################################
# Configure the following variables according to your AVR.
# Program the device with
#     make fuse    			# to set the clock generator, boot section size etc.
#     make flash   			# to load the boot loader into flash
#     make lock    			# to protect the boot loader from overwriting
#     make disablereset		# for ATtiny85 target - to use external reset line for IO (CAUTION: this is not easy to enable again, see README) 

F_CPU = 16500000


#check OS
#TODO - linux

ifeq ($(OS),Windows_NT)
	#Windows
	ARDUINOAPPDIR = C:/bin/arduino/
	AVRDIR		= $(ARDUINOAPPDIR)/hardware/tools/avr
	#i dont see AVRINCLUDE used

else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
	#linux
		ARDUINOAPPDIR = ../arduino/
		AVRDIR		= $(ARDUINOAPPDIR)/hardware/tools/avr
		AVRINCLUDE	= $(ARDUINOAPPDIR)/hardware/tools/avr/avr/include
    endif
    ifeq ($(UNAME_S),Darwin)
		#OSX 
		ARDUINOAPPDIR = /Applications/Arduino.app
		AVRDIR		= $(ARDUINOAPPDIR)/Contents/Java/hardware/tools/avr
		AVRINCLUDE	= $(ARDUINOAPPDIR)/Contents/Java/hardware/tools/avr/avr/include
    endif
endif
AVRBIN		= $(AVRDIR)/bin

DEVICE = attiny85

# hexadecimal address for bootloader section to begin. To calculate the best value:
# - make clean; make main.hex; ### output will list data: 2124 (or something like that)
# - for the size of your device (8kb = 1024 * 8 = 8192) subtract above value 2124... = 6068
# - How many pages in is that? 6068 / 64 (tiny85 page size in bytes) = 94.8125
# - round that down to 94 - our new bootloader address is 94 * 64 = 6016, in hex = 1780
BOOTLOADER_ADDRESS = 180B

LOCKOPT = -U lock:w:0x2f:m

#PROGRAMMER contains AVRDUDE options to address your programmer
PROGRAMMER = -c stk500v1
SERIAL = COM8
BAUD = 19200
FUSEOPT_t85 = -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m
FUSEOPT_t85_DISABLERESET = -U lfuse:w:0xe1:m -U efuse:w:0xfe:m -U hfuse:w:0x5d:m

FUSEOPT = $(FUSEOPT_t85)

#where  is 8bitmixtape core?
#you will have to adapt to your system, after you install the 8BitMixtape in arduino
ARDUINO_BOARD_CORE = C:\Users\user\AppData\Local\Arduino15\packages\8BitMixtape\hardware\avr\0.0.28\cores\tiny
ARDUINO_BOARD_PINS = C:\Users\user\AppData\Local\Arduino15\packages\8BitMixtape\hardware\avr\0.0.28\variants\tinyX5
#ARDUINO_BOARD_CORE = /Users/xcorex/Library/Arduino15/packages/ATTinyCore/hardware/avr/1.0.6/cores/tiny
#ARDUINO_BOARD_PINS = /Users/xcorex/Library/Arduino15/packages/ATTinyCore/hardware/avr/1.0.6/variants/tinyX5
ARDUINO_BOARD_INCLUDES = -I$(ARDUINO_BOARD_CORE) -I$(ARDUINO_BOARD_PINS) -I../TinyAudioBoot

# Tools:
AVRDUDE = $(AVRBIN)/avrdude $(PROGRAMMER) -p $(DEVICE) -b$(BAUD)  -P$(SERIAL) -C $(AVRDIR)/etc/avrdude.conf
CC = $(AVRBIN)/avr-gcc
CPP = $(AVRBIN)/avr-g++
AVROBJCOPY= $(AVRBIN)/avr-objcopy
AVROBJDUMP= $(AVRBIN)/avr-objdump
AVRSIZE= $(AVRBIN)/avr-size

# Options:
DEFINES = -DBOOTLOADER_ADDRESS=0x$(BOOTLOADER_ADDRESS) -DARDUINO=10801 -DARDUINO_AVR_COCOMAKE7 -DARDUINO_ARCH_AVR -DF_CPU=$(F_CPU)
CPPFLAGS = -c -g -Os -w -std=gnu++11 -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -I. -mmcu=$(DEVICE) $(DEFINES) $(ARDUINO_BOARD_INCLUDES) 
CFLAGS = -c -g -Os -w -std=gnu11 -ffunction-sections -fdata-sections -MMD -I. -mmcu=$(DEVICE) $(DEFINES) $(ARDUINO_BOARD_INCLUDES) 
LDFLAGS = -Wl,--relax,--gc-sections -Wl,--section-start=.text=$(BOOTLOADER_ADDRESS),-Map=main.map

OBJECTS = main.o


# symbolic targets:
all: main.hex

## Compile c++ files

%.o : %.ino
	$(CPP) $(CPPFLAGS) -x c++ -c $< -o $@ -Wa,-ahls=$<.lst

.cpp.o:
	$(CPP) $(CPPFLAGS) -x c++ -c $< -o $@ -Wa,-ahls=$<.lst

## Compile c files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ -Wa,-ahls=$<.lst


## Compile S files
.S.o:
	$(CC) $(CPPFLAGS) -c -g -x assembler-with-cpp $< -o $@

.c.s:
	@$(CC) $(CFLAGS) -S $< -o $@

.cpp.s:
	$(GCC) $(CPPFLAGS) -S $< -o $@

flash:	all
	$(AVRDUDE) $(FUSEOPT) -b 19200  -PCOM8
	$(AVRDUDE) -Uflash:w:main.hex:i -b 19200  -PCOM8

readflash:
	$(AVRDUDE) -U flash:r:read.hex:i

fuse:
	$(AVRDUDE) $(FUSEOPT) -b 19200  -PCOM8
	
disablereset:
	$(AVRDUDE) $(FUSEOPT_t85_DISABLERESET)

lock:
	$(AVRDUDE) $(LOCKOPT)

read_fuses:
	$(UISP) --rd_fuses -b 19200  -PCOM8

clean:
	rm -f main.hex main.bin *.o main.s main.cpp.lst TinyAudioBoot.ino.lst main.map main.d TinyAudioBoot.d SendOnlySoftwareSerial.cpp.lst SendOnlySoftwareSerial.d

winClean:
	del /f main.hex main.bin *.o main.s main.cpp.lst TinyAudioBoot.ino.lst main.map main.d TinyAudioBoot.d
# file targets:

#link files
main.bin:	$(OBJECTS)
	$(CC) -w -Os -Wl,--gc-sections -mmcu=$(DEVICE) -o main.bin $(OBJECTS) $(LDFLAGS)

main.hex:	main.bin
	$(AVROBJCOPY) -j .text -j .data -O ihex main.bin main.hex 
	@echo Size of binary hexfile. Use the "data" size to calculate the bootloader address
	@echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	$(AVRSIZE) main.hex

disasm:	main.bin
	$(AVROBJDUMP) -d main.bin