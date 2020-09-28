#!/bin/bash
#this script will download this version and place it inside your project. 

#download arduino 
wget https://downloads.arduino.cc/arduino-1.8.13-linux64.tar.xz
tar -xf arduino-1.8.13-linux64.tar.xz
#just renaming the thing
mv arduino-1.8.13-linux64 arduino/

#to have desktop icons and stuff. 
#cd arduino && ./install.sh
