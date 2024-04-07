#!/bin/bash


which sox > /dev/null
if [[ ! $? -eq 0 ]]; then
    sudo apt-get install -y sox
fi

OUTFILE=/tmp/record/record.wav
rm -f ${OUTFILE}
sox $@ ${OUTFILE}
