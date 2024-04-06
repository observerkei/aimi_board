#!/bin/bash

which edge-tts > /dev/null
if [ ! $? -eq 0 ]; then
	pip3 install edge-tts
fi

which mplayer > /dev/null
if [[ ! $? -eq 0 ]]; then
	sudo apt-get install -y mplayer
    sudo usermod -a -G audio `whoami`
    sudo usermod -a -G video `whoami`
    touch /tmp/audio_no_restart
    echo "add user to audio and video mod, need restart. "
fi

use_sudo=0
if [[ -f "/tmp/audio_no_restart" ]]; then
    use_sudo=1
fi

default_speak_mode=zh-CN-XiaoxiaoNeural
speak_file=/tmp/audio.mp3

edge-tts --text $1 --write-media ${speak_file}  -v ${default_speak_mode}
# 如果添加 usermod 后, 则需要重启设备才能不加 sudo 使用
if [[ use_sudo -eq 0 ]]; then
    mplayer ${speak_file}
else
    sudo mplayer ${speak_file}
fi