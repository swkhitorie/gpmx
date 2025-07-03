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
PROTOTYPE=$SCRIPTDIR/../boards/fmu-v2/fmuv2-firmware.prototype
fi

if [ $FMUTYPE = "fmuv6c" ]; then
PROTOTYPE=$SCRIPTDIR/../boards/holybro-fmu-v6c/fmuv6c-firmware.prototype
fi

MKFW=$SCRIPTDIR/px_mkfw.py

python $MKFW \
    --prototype $PROTOTYPE \
    --image $FMUBIN \
    > ./bin/$FMUOUTPUT

python $SCRIPTDIR/px_uploader.py --port $SERIAL_PORTS ./bin/$FMUOUTPUT

# windows:
# busybox.exe bash .\tools\px_upload.sh .\bin\px4_fmu-v2.bin fmuv6c COM13

# linux:
# ./tools/px_upload.sh ./bin/px4_fmu-v2.bin fmuv6c /dev/ttyACM0
