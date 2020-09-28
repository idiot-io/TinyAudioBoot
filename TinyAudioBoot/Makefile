### 
### This is based on example Makefile and it MUST be configured to suit your needs.
### For detailed explanations about all of the available options, please refer
### to https://github.com/sudar/Arduino-Makefile/blob/master/arduino-mk-vars.md
### Original project where this Makefile comes from: https://github.com/WeAreLeka/Bare-Arduino-Project

#####################################
##### Tinybootloader specials #######
#####################################

# comments from budi
# hexadecimal address for bootloader section to begin. To calculate the best value:
# - make clean; make main.hex; ### output will list data: 2124 (or something like that)
# - for the size of your device (8kb = 1024 * 8 = 8192) subtract above value 2124... = 6068
# - How many pages in is that? 6068 / 64 (tiny85 page size in bytes) = 94.8125
# - round that down to 94 - our new bootloader address is 94 * 64 = 6016, in hex = 1780
BOOTLOADER_ADDRESS = 0x1BC0

## ??
LOCKOPT = -U lock:w:0x2f:m

#F_CPU = 16500000
F_CPU 		   = 8000000L
DEFINES 	   = -DBOOTLOADER_ADDRESS=0x$(BOOTLOADER_ADDRESS)
CPPFLAGS = -g -Os -w -W -ffunction-sections -fdata-sections  -MMD -I.  $(DEFINES) $(ARDUINO_BOARD_INCLUDES) 
#PROGRAMMER contains AVRDUDE options to address your programmer
PROGRAMMER = -c USBasp

FUSEOPT_t85 = -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m
FUSEOPT_t85_DISABLERESET = -U lfuse:w:0xe1:m -U efuse:w:0xfe:m -U hfuse:w:0x5d:m

FUSEOPT = $(FUSEOPT_t85)


ARDUINO_BOARD_INCLUDES = -I$(ARDUINO_BOARD_CORE) -I$(ARDUINO_BOARD_PINS) -I../TinyAudioBoot

##################################################################
##################################################################
##################################################################

### PROJECT_DIR
### This is the path to where you have created/cloned your project
PROJECT_DIR       = /mnt/c/dev/TinyAudioBoot/TinyAudioBoot
 
### ARDMK_DIR
### Path to the Arduino-Makefile directory.
##if you did 'git submodule updtae --init' you will have this
ARDMK_DIR         = $(PROJECT_DIR)/../Arduino-mk
					
### ARDUINO_DIR
### Path to the Arduino application and resources directory.
ARDUINO_DIR       = /opt/arduino

### USER_LIB_PATH
### Path to where the your project's libraries are stored.
USER_LIB_PATH    :=  $(PROJECT_DIR)/lib

### BOARD_TAG
### It must be set to the board you are currently using. (i.e uno, mega2560, etc.)
ALTERNATE_CORE_PATH = $(PROJECT_DIR)/../ATTinyCore/avr/
BOARD_TAG 	   = attinyx5
BOARD_SUB 	   = 85
VARIANT        = tinyX5
ARDUINO_BOARD_INCLUDES = -I$(ARDUINO_BOARD_CORE) -I$(ARDUINO_BOARD_PINS) -I../TinyAudioBoot
### MONITOR_BAUDRATE
### It must be set to Serial baudrate value you are using.
MONITOR_BAUDRATE  = 115200

### AVR_TOOLS_DIR
### Path to the AVR tools directory such as avr-gcc, avr-g++, etc.
### On OS X with `homebrew`:
AVR_TOOLS_DIR     = /usr/

### AVRDUDE
### Path to avrdude directory.
AVRDUDE          = /usr/bin/avrdude

### CFLAGS_STD
### Set the C standard to be used during compilation. Documentation (https://github.com/WeAreLeka/Arduino-Makefile/blob/std-flags/arduino-mk-vars.md#cflags_std)
CFLAGS_STD        = -std=gnu11

### CXXFLAGS_STD
### Set the C++ standard to be used during compilation. Documentation (https://github.com/WeAreLeka/Arduino-Makefile/blob/std-flags/arduino-mk-vars.md#cxxflags_std)
CXXFLAGS_STD      = -std=gnu++11

### CXXFLAGS
### Flags you might want to set for debugging purpose. Comment to stop.
CXXFLAGS         += -pedantic -Wall -Wextra

### MONITOR_PORT
### The port your board is connected to. Using an '*' tries all the ports and finds the right one.
MONITOR_PORT      = /dev/tty.ttyACM*

### CURRENT_DIR
### Do not touch - used for binaries path
CURRENT_DIR       = $(shell basename $(CURDIR))

### OBJDIR
### This is where you put the binaries you just compile using 'make'
OBJDIR            = $(PROJECT_DIR)/bin/$(BOARD_TAG)/$(CURRENT_DIR)

### Do not touch - the path to Arduino.mk, inside the ARDMK_DIR
include $(ARDMK_DIR)/Arduino.mk
