#!/usr/bin/env bash

EXEDIR=`pwd`
BASEDIR=$(dirname $0)
SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
SYSTYPE=`uname -s`

# $1: fmu bin
# $2: serial usb port

UPLOADBIN=$1
SERIAL_PORTS=$2

python3 $SCRIPTDIR/ymdm_uploader.py --port $SERIAL_PORTS $UPLOADBIN

# ./tools/ymdm_upload.sh <bin> /dev/ttyUSB0

