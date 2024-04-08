#!/bin/bash

sudo apt-get install -y python3 python3-pip sox

which mplayer > /dev/null
if [[ ! $? -eq 0 ]]; then
    sudo apt-get install -y mplayer
    sudo usermod -a -G audio `whoami`
    sudo usermod -a -G video `whoami`
    sudo usermod -a -G input `whoami`
    touch /tmp/driver_no_restart
    echo "add user to audio and video mod, need restart. "
fi

echo 'openai
azure-cognitiveservices-speech
evdev
httpx[socks]
edge-tts' > /tmp/requirements.txt

pip3 install -r /tmp/requirements.txt
