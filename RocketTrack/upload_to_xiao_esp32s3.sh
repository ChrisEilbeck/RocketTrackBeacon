#!/bin/bash

#VERBOSE=--verbose
BOARD=esp32:esp32:XIAO_ESP32S3

PORT=/dev/ttyACM0

arduino-cli upload ${VERBOSE} --fqbn ${BOARD} --port ${PORT}

