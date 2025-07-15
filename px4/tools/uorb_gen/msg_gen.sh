#!/bin/sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

PYTHON_EXECUTABLE=python3
UORB_GEN_SCRIPT=${SCRIPT_DIR}/tools/px_generate_uorb_topic_files.py

UORB_MSG_PATH=$1
UORB_GENERATE_PATH=$2

UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/uorb/
if [ $# -ge 3 ]; then
    if [ $3 = "posix" ]; then
        UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/uorb_posix/
    fi
fi

if [ ! -d ${UORB_GENERATE_PATH} ]; then
    mkdir ${UORB_GENERATE_PATH}
fi

if [ ! -d ${UORB_MSG_PATH} ]; then
    echo "uorb msg path not exist"
    exit 1
fi

# generate path
UORB_HEADER_OUTPUT=${UORB_GENERATE_PATH}/uorb/topics/
UORB_SOURCES_OUTPUT=${UORB_GENERATE_PATH}/msg/topics_sources/

UORB_HEADER_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/headers/
UORB_SOURCES_TEMPORARY_DIR=${UORB_GENERATE_PATH}/msg/tmp/sources/

echo "Generating uorb msg..."

UORB_MSG_FILES_NOT_EMPTY=0
UORB_MSG_FILES=""
for file in $(ls ${UORB_MSG_PATH})
do
    if [ "${file##*.}" = "msg" ]; then
        UORB_MSG_FILES="${UORB_MSG_FILES}${UORB_MSG_PATH}${file} "
        UORB_MSG_FILES_NOT_EMPTY=1
    fi
done

if [ ${UORB_MSG_FILES_NOT_EMPTY} -eq 1 ]; then
    # generate
    ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --headers \
            -f ${UORB_MSG_FILES} \
            -i ${SCRIPT_DIR}/ \
            -o ${UORB_HEADER_OUTPUT} \
            -e ${UORB_GEN_TEMPLATE} \
            -t ${UORB_HEADER_TEMPORARY_DIR} \
            -q

    ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --sources \
            -f ${UORB_MSG_FILES} \
            -i ${SCRIPT_DIR}/ \
            -o ${UORB_SOURCES_OUTPUT} \
            -e ${UORB_GEN_TEMPLATE} \
            -t ${UORB_SOURCES_TEMPORARY_DIR} \
            -q

    echo "Generating uorb msg completed"
else
    echo "No Any msg files"
fi

# ./px4/tools/uorb_gen/msg_gen.sh ./px4/msg/ ./px4/build <posix>
