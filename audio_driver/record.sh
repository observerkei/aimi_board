#!/bin/bash

DURATION=3

OUTPUT_DIR="/tmp/record"
if [[ ! -d "${OUTPUT_DIR}" ]]; then
    mkdir -p ${OUTPUT_DIR}
fi


DEVICE=$1
if [[ -z "${DEVICE}" ]]; then
    DEVICE="hw:0,0"
fi

FILENAME=$2
if [[ -z "${FILENAME}" ]]; then
    FILENAME=${OUTPUT_DIR}/record.wav
fi

rm -f ${FILENAME}

echo "start record"
arecord -D $DEVICE -f cd -c 1 -r 44100 -d $DURATION ${FILENAME}
echo "create ${FILENAME}"
