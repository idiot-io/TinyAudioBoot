## How to flash example for AVR-Dragon:  
Flash the file:  
```
avrdude -v -pt85 -c dragon_isp -Pusb -b115200 -Uflash:w:AudioBootAttiny_AudioPB3_PB1.hex 
```

set the fuses with reset enabled:
```
avrdude -v -pt85 -c dragon_isp -Pusb -b115200 -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

# How to flash example using bootloadbuilder makefile

## before we start
  * [install arduino](https://www.arduino.cc/en/Main/Software) 

  * d/l the [8bitmixtape](https://github.com/8BitMixtape/8Bit-Mixtape-NEO) core  
    "Add this to your additional hardware manager:"  
    ``http://8bitmixtape.github.io/package_8bitmixtape_index.json``

  * in  ``bootloaderbuild/Makefile`` to you
    * change arduino directory ``ARDUINOAPPDIR``   
      notice for windows/unix/osX OS the location to change is diffrent.  
      line #[27,35,41] respectivly  

    * adapt the location of your tiny cores.   
      see lines #[70.71]

## symbolic link
make sure you have a working symbolic link between ``TinyAudioBoot\TinyAudioBoot.c`` and ``bootloaderbuild\main.cpp``  

**in linux**  
to avoid shit make sure its full path and not relative. 
```
#ln TARGET LINK_NAME
ln /opt/TinyAudioBoot/TinyAudioBoot/TinyAudioBoot.c /opt/TinyAudioBoot/bootloaderbuild/main.cpp 
ln /opt/TinyAudioBoot/TinyAudioBoot/EEPROM.h /opt/TinyAudioBoot/bootloaderbuild/EEPROM.h 
```

**in windows**   
powershell something like this  
must be done in Admin mode. and no relative links.  
notice its reversed then unix, first the link to create then the target.
```
New-Item -ItemType SymbolicLink -Path C:\dev\TinyAudioBoot\bootloaderbuild\main.cpp -Target C:\dev\TinyAudioBoot\TinyAudioBoot\TinyAudioBoot.c
New-Item -ItemType SymbolicLink -Path C:\dev\TinyAudioBoot\bootloaderbuild\EEPROM.h -Target C:\dev\TinyAudioBoot\TinyAudioBoot\EEPROM.h
```
if oyou yu want5 to experimnte with new skipper mode, delete the old symlink and try
```
New-Item -ItemType SymbolicLink -Path C:\dev\TinyAudioBoot\bootloaderbuild\main.cpp -Target C:\dev\TinyAudioBoot\c_src\TinyAudioBoot.c
New-Item -ItemType SymbolicLink -Path C:\dev\TinyAudioBoot\bootloaderbuild\EEPROM.h -Target C:\dev\TinyAudioBoot\c_src\EEPROM.h
```

## fix address size


run
```
cd bootloaderbuild
make clean # in windows run ''make winClean''
make flash
```

if all is well scroll back and find the size of your bootlaoder hex file.  
it will be written in the console.  
```
   text    data     bss     dec     hex filename
      0    1006       0    1006     3ee main.hex
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

**error**   
if you get   
```main.bin section `.text' is not within region `text'```  

change your ``#define BOOTLOADER_ADDRESS``  to  lower value like ``180B`` and run again

# compile and flash

now, again
```
make flash
```

we are done wth flash. 


wav tested with  
``build/flexiTinyAudio_NeoButton.ino.wav``


