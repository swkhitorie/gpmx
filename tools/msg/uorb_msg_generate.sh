#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ $# -ne 2 ]; then
    echo "cmd: uorb_msg_generath.sh <msg_path> <generate_path>"
    exit 1
fi

PYTHON_EXE=python
if [ $(uname) == "Linux" ]; then
    PYTHON_EXE=python3
fi

UORB_MSG_PATH=$1
UORB_GENERATE_PATH=$2
UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/

if [ ! -d ${UORB_MSG_PATH} ]; then
    echo "uorb msg path not exist"
    exit 1
fi

# if [ ! -d ${UORB_GENERATE_PATH} ]; then
#     mkdir -p "${UORB_GENERATE_PATH}"
# fi

UORB_GEN_SCRIPT=${SCRIPT_DIR}/tools/px_generate_uorb_topic_files.py

# generate path
UORB_HEADER_OUTPUT=${UORB_GENERATE_PATH}/uorb/topics/
UORB_SOURCES_OUTPUT=${UORB_GENERATE_PATH}/msg/topics_sources/

UORB_HEADER_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/headers/
UORB_SOURCES_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/sources/

UORB_MSG_NUM=0
UORB_MSG_FILES=""
UORB_MSG_SNAKE_FILES=""
for file in $(ls ${UORB_MSG_PATH})
do
    if [ "${file##*.}" = "msg" ]; then
        UORB_MSG_FILES="${UORB_MSG_FILES}${UORB_MSG_PATH}${file} "
        let UORB_MSG_NUM=UORB_MSG_NUM+1
    fi
done

# for file in "${UORB_MSG_PATH}"*.msg; do
#     [ -e "$file" ] || continue

#     abs_file=$(realpath "$file")
#     dir_part=$(dirname "$abs_file")
#     base_name=$(basename "$abs_file" .msg)

#     snake_name=$(echo "$base_name" | sed -E '
#         s/([a-z0-9])([A-Z])/\1_\2/g;
#         s/([A-Z])([A-Z][a-z])/\1_\2/g
#     ' | tr '[:upper:]' '[:lower:]')
#     new_abs_file="${dir_part}/${snake_name}.msg"
#     UORB_MSG_SNAKE_FILES="${UORB_MSG_SNAKE_FILES}${new_abs_file} "
# done

if [ $UORB_MSG_NUM -eq 0 ]; then
    echo "No Any msg files"
    exit 1
fi

echo "Generating uorb msg...(total: ${UORB_MSG_NUM})"
# generate
${PYTHON_EXE} ${UORB_GEN_SCRIPT} \
        --headers \
        -f ${UORB_MSG_FILES} \
        -i ${UORB_MSG_PATH}/ \
        -o ${UORB_HEADER_OUTPUT} \
        -e ${UORB_GEN_TEMPLATE}

${PYTHON_EXE} ${UORB_GEN_SCRIPT} \
        --sources \
        -f ${UORB_MSG_FILES} \
        -i ${UORB_MSG_PATH}/ \
        -o ${UORB_SOURCES_OUTPUT} \
        -e ${UORB_GEN_TEMPLATE}

echo "Generating uorb msg completed"

# ./tools/msg/uorb_msg_generate.sh ./msg/ ./build
