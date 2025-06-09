#!/bin/bash

#VERBOSE=--verbose
BOARD=esp32:esp32:heltec_wireless_tracker

arduino-cli lib uninstall SD

rm data/*~ 2>/dev/null

##mkspiffs -c data -b 4096 -p 256 -s 0x170000 rockettrack.spiffs.bin
##mkspiffs -c data -b 4096 -p 256 -s 0x160000 rockettrack.spiffs.bin

arduino-cli compile --fqbn ${BOARD} ${VERBOSE}

