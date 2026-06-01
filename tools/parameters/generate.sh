#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PYTHON_EXE=python
if [ $(uname) == "Linux" ]; then
    PYTHON_EXE=python3
fi

OUTPUT_XML_PATH=${SCRIPT_DIR}/../../build/parameters/parameters.xml
OUTPUT_JSON_PATH=${SCRIPT_DIR}/../../build/parameters/parameters.json
OUTPUT_DEST_PATH=${SCRIPT_DIR}/../../build/parameters


BOARDNAME="default"
INJECT_XML=${SCRIPT_DIR}/parameters_injected.xml

${PYTHON_EXE} ${SCRIPT_DIR}/px_process_params.py \
		--src-path ${SCRIPT_DIR}/../../apps/libs/modules/sensors \
		--xml ${OUTPUT_XML_PATH} \
		--json ${OUTPUT_JSON_PATH} \
		--compress \
		--inject-xml ${INJECT_XML} \
		--board ${BOARDNAME} \
		#--verbose

${PYTHON_EXE} ${SCRIPT_DIR}/px_generate_params.py \
		--xml ${OUTPUT_XML_PATH} \
        --dest ${OUTPUT_DEST_PATH}
