#!/bin/bash

arduino-cli lib install SD

arduino-cli lib install "adafruit spiflash"
arduino-cli lib install "sdfat - adafruit fork"

rm configdata/*~ 2>/dev/null
mkspiffs -c configdata -b 4096 -p 256 -s 0x170000 rockettrack.spiffs.bin

arduino-cli compile \
	--fqbn adafruit:samd:adafruit_feather_m0 \
	--verbose \
	RocketTrack.ino 
