#!/bin/bash

#pip install esptool

port=/dev/tty.wchusbserial620

python /Users/wanghui/Library/Python/2.7/bin/esptool.py --port $port erase_flash
