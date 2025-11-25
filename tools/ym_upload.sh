#!/usr/bin/env bash

EXEDIR=`pwd`
BASEDIR=$(dirname $0)
SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
SYSTYPE=`uname -s`

# $1: serial usb port
# $2: fmu bin

UPLOADBIN=$2
SERIAL_PORTS=$1

python $SCRIPTDIR/ym_uploader.py $SERIAL_PORTS $UPLOADBIN

# ym_upload.sh <bin> /dev/ttyUSB0

