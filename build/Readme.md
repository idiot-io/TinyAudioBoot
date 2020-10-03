## How to flash example for AVR-Dragon:  
Flash the file:  
```
avrdude -v -pt85 -c dragon_isp -Pusb -b115200 -Uflash:w:AudioBootAttiny_AudioPB3_PB1.hex 
```

set the fuses with reset enabled:
```
avrdude -v -pt85 -c dragon_isp -Pusb -b115200 -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

## How to flash example using bootloadbuilder makefile

make sure you have a working symbolic link between ``TinyAudioBoot\TinyAudioBoot.c`` and ``bootloaderbuild\main.cpp``  

**in linux**
```
#ln TARGET LINK_NAME
ln TinyAudioBoot/TinyAudioBoot/TinyAudioBoot.c /opt/TinyAudioBoot/bootloaderbuild/main.cpp /opt/
```

**in windows**   
powershell something like this  
notice its reversed then unix, first the link to create then the target.
```
New-Item -ItemType SymbolicLink -Path C:\dev\TinyAudioBoot\bootloaderbuild\main.cpp -Target C:\dev\TinyAudioBoot\TinyAudioBoot\TinyAudioBoot.c
```

adapt  ``bootloaderbuild/Makefile`` to you arduino directory ``ARDUINOAPPDIR``   
notice for windows/unix/osX OS the location to change is diffrent.  
line #[27,35,41] respectivly  

run
```
cd bootloaderbuild
make flash
```

if all is well go back and find the size of your bootlaoder hex file. it will be written in the console. 
```

```
then do the calculation mentioned in the makefile

```
output will list data: 2124 (or something like that)
# - for the size of your device (8kb = 1024 * 8 = 8192) subtract above value 2124... = 6068
# - How many pages in is that? 6068 / 64 (tiny85 page size in bytes) = 94.8125
# - round that down to 94 - our new bootloader address is 94 * 64 = 6016, in hex = 1780
```
and feed it into ``TinyAudioBoot\TinyAudioBoot.c``  
there is a var there. 
``#define BOOTLOADER_ADDRESS     0x1C00``  
adapt and reflash  
```
make flash
```

we are done wth flash. 


wav tested with  
``build/flexiTinyAudio_NeoButton.ino.wav``


