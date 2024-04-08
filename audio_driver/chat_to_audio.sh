#!/bin/bash

which edge-tts > /dev/null
if [ ! $? -eq 0 ]; then
	pip3 install edge-tts
fi

default_speak_mode=zh-CN-XiaoxiaoNeural

speak_file=/tmp/audio.mp3
if [[ ! -z "$1" ]]; then
    speak_file=$1
    shift
fi

speak_txt=/tmp/speak.txt

rm -f ${speak_file} ${speak_txt}

echo "$@" > ${speak_txt}

edge-tts -f ${speak_txt} --write-media ${speak_file} -v ${default_speak_mode}