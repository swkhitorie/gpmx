#!/usr/bin/env python3

# for python2.7 compatibility
from __future__ import print_function

import sys
import argparse
import binascii
import serial
import socket
import struct
import json
import zlib
import base64
import time
import array
import os

from sys import platform as _platform

# Detect python version
if sys.version_info[0] < 3:
    runningPython3 = False
else:
    runningPython3 = True

class firmware(object):
    '''Loads a firmware file'''
    image = bytes()
    image_size = 0

    def __init__(self, path):

        # read the file
        f = open(path, "rb")
        self.image_size = os.path.getsize(path)
        self.image = f.read()
        f.close()

    def crc(self):

        wCRCin = 0x0000
        wCPoly = 0x1021
        size = self.image_size
        idx = 0
        while size:
            wCRCin ^= (self.image[idx] << 8)
            idx += 1
            size -= 1
            for num in range(1,8+1):
                if wCRCin & 0x8000:
                    wCRCin = (wCRCin << 1) ^ wCPoly
                else:
                    wCRCin = wCRCin << 1
        return wCRCin


def main():

    # Parse commandline arguments
    parser = argparse.ArgumentParser(description="Firmware uploader for the YModem Receiver.")
    parser.add_argument('--port', action="store", required=True, help="Comma-separated list of serial port(s)")
    parser.add_argument('--baud-bootloader', action="store", type=int, default=115200, help="Baud rate of the serial port (default is 115200) when communicating with bootloader, only required for true serial ports.")
    parser.add_argument('firmware', action="store", help="Firmware file to be uploaded")
    args = parser.parse_args()

    # We need to check for pyserial because the import itself doesn't
    # seem to fail, at least not on macOS.
    pyserial_installed = False
    try:
        if serial.__version__:
            pyserial_installed = True
    except:
        pass

    try:
        if serial.VERSION:
            pyserial_installed = True
    except:
        pass

    if not pyserial_installed:
        print("Error: pyserial not installed!")
        print("")
        print("You may need to install it using:")
        print("    pip3 install --user pyserial")
        print("")
        sys.exit(1)

    # print(args.firmware)
    # print(args.port)
    # print(args.baud_bootloader)

    print("hello")
    fw = firmware(args.firmware)
    print(fw.image_size)
    print(fw.crc())


if __name__ == '__main__':
    main()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
