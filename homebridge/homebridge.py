#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os;
import commands as cmd;
if __name__=="__main__":
    rs=cmd.getoutput("ps -ef|grep homebridge|grep -v grep|wc -l")
    if(int(rs)==3):
        print("Homebridge is running!")
    else:
        print("Starting homebridge...")
        os.system("/etc/init.d/homebridge start")
