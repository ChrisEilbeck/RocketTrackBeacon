#!/bin/bash

#VERBOSE=--verbose
BOARD=esp32:esp32:t-beam

PORT=/dev/ttyUSB1
PORT=/dev/ttyACM0

arduino-cli upload ${VERBOSE} --fqbn ${BOARD} --port ${PORT}

rm data/*~ 2>/dev/null

mkspiffs -c data -b 4096 -p 256 -s 0x160000 rockettrack.spiffs.bin

`find ~/.arduino15/packages/esp32/tools/esptool_py/ -name esptool -print` --chip esp32 \
	--port ${PORT} \
	--baud 921600 \
	write_flash -z 0x290000 rockettrack.spiffs.bin

#python3 `find ~/.arduino15/packages/esp32/tools/esptool_py/ -name esptool.py -print` --chip esp32 \
#    --port ${PORT} \
#    --baud 921600 \
#    write_flash -z 0x290000 rockettrack.spiffs.bin

#esptool \
#	--chip esp32 \
#	--port ${PORT} \
#	--baud 921600 \
#	write_flash -z 0x290000 rockettrack.spiffs.bin

