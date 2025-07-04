#!/bin/sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

PYTHON_EXECUTABLE=python

UORB_GEN_SCRIPT=${SCRIPT_DIR}/tools/px_generate_uorb_topic_files.py
UORB_GEN_TEMPLATE=${SCRIPT_DIR}/templates/uorb/

UORB_MSG_PATH=${SCRIPT_DIR}/../../msg/
UORB_GENERATE_PATH=${SCRIPT_DIR}/../../build/

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

for file in $(ls ${UORB_MSG_PATH})
do 
    if [ "${file##*.}" = "msg" ]; then
        # generate
        ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --headers \
                -f ${UORB_MSG_PATH}/${file} \
                -i ${SCRIPT_DIR}/ \
                -o ${UORB_HEADER_OUTPUT} \
                -e ${UORB_GEN_TEMPLATE} \
                -t ${UORB_HEADER_TEMPORARY_DIR} >> /dev/null

        ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --sources \
                -f ${UORB_MSG_PATH}/${file} \
                -i ${SCRIPT_DIR}/ \
                -o ${UORB_SOURCES_OUTPUT} \
                -e ${UORB_GEN_TEMPLATE} \
                -t ${UORB_SOURCES_TEMPORARY_DIR} >> /dev/null
    fi
done

echo "Generating uorb msg completed"

# # generate example
# ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --headers \
#         -f ${SCRIPT_DIR}/msg/sensor_accel.msg \
#         -i ${SCRIPT_DIR}/ \
#         -o ${UORB_HEADER_OUTPUT} \
#         -e ${UORB_GEN_TEMPLATE} \
#         -t ${UORB_HEADER_TEMPORARY_DIR}

# ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --sources \
#         -f ${SCRIPT_DIR}/msg/sensor_accel.msg \
#         -i ${SCRIPT_DIR}/ \
#         -o ${UORB_SOURCES_OUTPUT} \
#         -e ${UORB_GEN_TEMPLATE} \
#         -t ${UORB_SOURCES_TEMPORARY_DIR}
