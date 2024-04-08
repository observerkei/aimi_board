#!/bin/bash

which mplayer > /dev/null
if [[ ! $? -eq 0 ]]; then
    sudo apt-get install -y mplayer
    sudo usermod -a -G audio `whoami`
    sudo usermod -a -G video `whoami`
    sudo usermod -a -G input `whoami`
    touch /tmp/driver_no_restart
    echo "add user to audio and video mod, need restart. "
fi

use_sudo=0
if [[ ! -f "/tmp/driver_no_restart" ]]; then
    use_sudo=1
fi

speak_file=/tmp/audio.mp3
if [[ ! -z "$1" ]]; then
    speak_file=$1
fi

# 如果添加 usermod 后, 则需要重启设备才能不加 sudo 使用
if [[ $use_sudo -eq 0 ]]; then
    mplayer ${speak_file}
else
    sudo mplayer ${speak_file}
fi