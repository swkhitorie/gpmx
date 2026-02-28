#!/bin/sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
if [ $# -ne 3 ]; then
    echo "cmd: uorb_msg_generath.sh <msg_path> <generate_path> <format:posix/px4>"
    exit 1
fi

PYTHON_EXE=python
if [ $(uname) == "Linux" ]
then
    PYTHON_EXE=python3
fi

UORB_MSG_PATH=$1
UORB_GENERATE_PATH=$2
UORB_FORMAT=$3

UORB_GEN_TEMPLATE="null"

if [ ${UORB_FORMAT} == "posix" ]; then
    echo "uorb posix format"
    UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/uorb_posix/
fi

if [ ${UORB_FORMAT} == "px4" ]; then
    echo "uorb px4 format"
    UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/uorb/
fi

if [ ${UORB_GEN_TEMPLATE} == "null" ]; then
    echo "please indicate generate format <format:posix/px4>"
    exit 1
fi

if [ ! -d ${UORB_MSG_PATH} ]; then
    echo "uorb msg path not exist"
    exit 1
fi

if [ ! -d ${UORB_GENERATE_PATH} ]; then
    mkdir ${UORB_GENERATE_PATH}
fi

UORB_GEN_SCRIPT=${SCRIPT_DIR}/tools/px_generate_uorb_topic_files.py

# generate path
UORB_HEADER_OUTPUT=${UORB_GENERATE_PATH}/uorb/topics/
UORB_SOURCES_OUTPUT=${UORB_GENERATE_PATH}/msg/topics_sources/

UORB_HEADER_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/headers/
UORB_SOURCES_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/sources/

UORB_MSG_NUM=0
UORB_MSG_FILES=""
for file in $(ls ${UORB_MSG_PATH})
do
    if [ "${file##*.}" = "msg" ]; then
        UORB_MSG_FILES="${UORB_MSG_FILES}${UORB_MSG_PATH}${file} "
        let UORB_MSG_NUM=UORB_MSG_NUM+1
    fi
done

if [ $UORB_MSG_NUM -eq 0 ]; then
    echo "No Any msg files"
    exit 1
fi

echo "Generating uorb msg..."
# generate
${PYTHON_EXE} ${UORB_GEN_SCRIPT} --headers \
        -f ${UORB_MSG_FILES} \
        -i ${SCRIPT_DIR}/ \
        -o ${UORB_HEADER_OUTPUT} \
        -e ${UORB_GEN_TEMPLATE} \
        -t ${UORB_HEADER_TEMPORARY_DIR} \
        -q

${PYTHON_EXE} ${UORB_GEN_SCRIPT} --sources \
        -f ${UORB_MSG_FILES} \
        -i ${SCRIPT_DIR}/ \
        -o ${UORB_SOURCES_OUTPUT} \
        -e ${UORB_GEN_TEMPLATE} \
        -t ${UORB_SOURCES_TEMPORARY_DIR} \
        -q

echo "Generating uorb msg completed"

# ./tools/uorb_gen/uorb_msg_generate.sh ./msg/ ./build posix
