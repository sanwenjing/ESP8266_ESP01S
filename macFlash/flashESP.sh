#!/bin/bash

#pip install esptool

port=/dev/tty.wchusbserial620
echo "Flashing "$1"......"
python /Users/wanghui/Library/Python/2.7/bin/esptool.py --port $port --baud 115200 write_flash --flash_size=detect 0 $1
