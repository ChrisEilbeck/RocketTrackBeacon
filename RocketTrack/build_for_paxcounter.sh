#!/bin/bash

VERBOSE=--verbose

##BOARD=esp32:esp32:t-beam:Revision=Radio_SX1276
BOARD=esp32:esp32:ttgo-lora32:Revision=TTGO_LoRa32_v21new

arduino-cli lib uninstall SD

rm data/*~ 2>/dev/null

arduino-cli compile --fqbn ${BOARD} ${VERBOSE}

