#!/bin/bash

mkdir -p binaries

##arduino-cli lib uninstall SD

##VERBOSE=--verbose

BOARD=esp32:esp32:t-beam:Revision=Radio_SX1276
arduino-cli compile --fqbn ${BOARD} ${VERBOSE}

mkdir -p binaries/T-Beam

find ~/.cache/arduino/sketches -name RocketTrack\*.bin -exec cp {} binaries/T-Beam \;

exit

BOARD=esp32:esp32:ttgo-lora32:Revision=TTGO_LoRa32_v21new
arduino-cli compile --fqbn ${BOARD} ${VERBOSE}

BOARD=esp32:esp32:XIAO_ESP32S3
arduino-cli compile --fqbn ${BOARD} ${VERBOSE}

BOARD=esp32:esp32:heltec_wireless_tracker
arduino-cli compile --fqbn ${BOARD} ${VERBOSE}

