#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ $# -ne 3 ]; then
    echo "cmd: uorb_msg_generath.sh <msg_path> <generate_path> <force_flag>"
    exit 1
fi

PYTHON_EXE=python
if [ $(uname) == "Linux" ]; then
    PYTHON_EXE=python3
fi

UORB_MSG_PATH=$1
UORB_GENERATE_PATH=$2
UORB_FORCE_GENERATE_FLAG=$3
UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/

if [ ! -d ${UORB_MSG_PATH} ]; then
    echo "[uorb] msg path not exist"
    exit 1
fi

UORB_GEN_SCRIPT=${SCRIPT_DIR}/tools/px_generate_uorb_topic_files.py

# generate path
UORB_HEADER_OUTPUT=${UORB_GENERATE_PATH}/uorb/topics/
UORB_SOURCES_OUTPUT=${UORB_GENERATE_PATH}/msg/topics_sources/

UORB_HEADER_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/headers/
UORB_SOURCES_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/sources/

need_generate_uorb() {
    local msg_src_dir="$1"
    local out_dir="$2"
    [ ! -d "$out_dir" ] && return 0
    # Find new .msg
    local latest_msg=$(find "$msg_src_dir" -name "*.msg" -type f -printf '%T@ %p\n' 2>/dev/null | sort -n | tail -1 | cut -d' ' -f2-)
    [ -z "$latest_msg" ] && return 1
    local msg_time=$(stat -c %Y "$latest_msg")
    # Find new msg_header
    local latest_h=$(find "$out_dir/uorb/topics" -name "*.h" -type f -printf '%T@ %p\n' 2>/dev/null | sort -n | tail -1 | cut -d' ' -f2-)
    [ -z "$latest_h" ] && return 0
    local gen_time=$(stat -c %Y "$latest_h")
    # If the .msg file is newer than the generated file
    [ $msg_time -gt $gen_time ] && return 0
    return 1
}

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

if [ $UORB_MSG_NUM -eq 0 ]; then
    echo "[uorb] no any msg file"
    exit 1
fi

if [ $UORB_FORCE_GENERATE_FLAG -eq 1 ] || need_generate_uorb "${UORB_MSG_PATH}" "${UORB_GENERATE_PATH}"; then
    if [ ! -d ${UORB_GENERATE_PATH} ]; then
        mkdir -p ${UORB_GENERATE_PATH}
    fi
else
    echo "[uorb] headers are up to date, skipping generation."
    exit 0
fi

echo "[uorb] msgs generating...(total: ${UORB_MSG_NUM})"

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

echo "[uorb] msgs generating completed"

exit 0

# ./tools/msg/uorb_msg_generate.sh ./msg/ ./build
