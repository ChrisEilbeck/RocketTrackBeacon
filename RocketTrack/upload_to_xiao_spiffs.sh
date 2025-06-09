#!/bin/bash

#VERBOSE=--verbose
BOARD=esp32:esp32:XIAO_ESP32S3

PORT=/dev/ttyACM0

rm data/*~ 2>/dev/null

mkspiffs -c data -b 4096 -p 256 -s 0x160000 rockettrack.spiffs.bin

`find ~/.arduino15/packages/esp32/tools/esptool_py/ -name esptool -print` --chip esp32 \
	--port ${PORT} \
	--baud 921600 \
	write_flash -z 0x290000 rockettrack.spiffs.bin

