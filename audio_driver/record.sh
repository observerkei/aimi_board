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


use_sudo=0
if [[ ! -f "/tmp/driver_no_restart" ]]; then
    use_sudo=1
fi

rm -f ${FILENAME}

echo "start record"

# 如果添加 usermod 后, 则需要重启设备才能不加 sudo 使用
if [[ $use_sudo -eq 0 ]]; then
    arecord -D $DEVICE -f cd -c 1 -r 44100 -d $DURATION ${FILENAME}
else
    sudo arecord -D $DEVICE -f cd -c 1 -r 44100 -d $DURATION ${FILENAME}
fi

echo "create ${FILENAME}"