#!/usr/bin/env bash

EXEDIR=`pwd`
BASEDIR=$(dirname $0)
SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
SYSTYPE=`uname -s`

# $1: fmu bin
# $2: fmu hardware type
# $3: serial usb port

FMUBIN=$1
FMUTYPE=$2
SERIAL_PORTS=$3
FMUOUTPUT=$(basename $FMUBIN .bin).px4

if [ $FMUTYPE = "fmuv2" ]; then
PROTOTYPE=$SCRIPTDIR/../boards/stm32/pixhawk_fmuv2/firmware.prototype
fi

if [ $FMUTYPE = "fmuv6x" ]; then
PROTOTYPE=$SCRIPTDIR/../boards/stm32/pixhawk_fmuv6x/firmware.prototype
fi

MKFW=$SCRIPTDIR/px_mkfw.py

PYTHON_EXE=python
if [ $(uname) == "Linux" ]
then
    PYTHON_EXE=python3
fi

$PYTHON_EXE $MKFW \
    --prototype $PROTOTYPE \
    --image $FMUBIN \
    > ./bin/$FMUOUTPUT

$PYTHON_EXE $SCRIPTDIR/px_uploader.py --port $SERIAL_PORTS ./bin/$FMUOUTPUT

# ./tools/px_upload.sh <user_bin_path> <fmuv2/fmuv6x> <port>
