#!/bin/bash

DEVICE="hw:0,0"
DURATION=5
OUTPUT_DIR="/tmp/record/"

FILENAME=$1
if [[ -z "${FILENAME}" ]]; then
    FILENAME=record
fi

mkdir -p ${OUTPUT_DIR}
echo "start record"
sudo arecord -D $DEVICE -f cd -c 1 -r 44100 -d $DURATION $OUTPUT_DIR/${FILENAME}.wav
echo "create $OUTPUT_DIR/${FILENAME}.wav"
