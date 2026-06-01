#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PYTHON_EXE=python
if [ $(uname) == "Linux" ]; then
    PYTHON_EXE=python3
fi

MAVLINK_PATH=${SCRIPT_DIR}/../../apps/libs/mavlink
GENERATE_PATH=$1
GENERATE_DIALOG=$2
if [ ! -d ${GENERATE_PATH} ]; then
    mkdir -p "${GENERATE_PATH}"
fi

# generate
python3 ${MAVLINK_PATH}/pymavlink/tools/mavgen.py \
    --lang=C --wire-protocol=2.0 \
    --output=${GENERATE_PATH} \
    ${MAVLINK_PATH}/message_definitions/v1.0/${GENERATE_DIALOG}.xml > ${GENERATE_PATH}/mavgen_${GENERATE_DIALOG}.log

echo "Generating mavlink ${GENERATE_DIALOG} completed"

# ./tools/mavlink/generate.sh ./build/mavlink common/development

