#!/bin/sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

PYTHON_EXECUTABLE=python

UORB_GEN_SCRIPT=${SCRIPT_DIR}/tools/tools/px_generate_uorb_topic_files.py
UORB_GEN_TEMPLATE=${SCRIPT_DIR}/tools/templates/uorb/


UORB_MSG_TEMPLATE_FILES=${SCRIPT_DIR}/msg/

# header generate path
UORB_HEADER_OUTPUT=${SCRIPT_DIR}/uorb_build/uorb/topics/
UORB_HEADER_TEMPORARY_DIR=${SCRIPT_DIR}/uorb_build/msg/tmp/headers/

# sources generate path
UORB_SOURCES_OUTPUT=${SCRIPT_DIR}/uorb_build/msg/topics_sources/
UORB_SOURCES_TEMPORARY_DIR=${SCRIPT_DIR}/uorb_build/msg/tmp/sources/


for file in $(ls ${SCRIPT_DIR}/msg)
do 
    if [ "${file##*.}" = "msg" ]; then
        # generate
        ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --headers \
                -f ${SCRIPT_DIR}/msg/${file} \
                -i ${SCRIPT_DIR}/ \
                -o ${UORB_HEADER_OUTPUT} \
                -e ${UORB_GEN_TEMPLATE} \
                -t ${UORB_HEADER_TEMPORARY_DIR} >> /dev/null

        ${PYTHON_EXECUTABLE} ${UORB_GEN_SCRIPT} --sources \
                -f ${SCRIPT_DIR}/msg/${file} \
                -i ${SCRIPT_DIR}/ \
                -o ${UORB_SOURCES_OUTPUT} \
                -e ${UORB_GEN_TEMPLATE} \
                -t ${UORB_SOURCES_TEMPORARY_DIR} >> /dev/null
    fi
done

# # generate
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
