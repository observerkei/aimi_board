#!/bin/bash

sudo apt-get install -y python3 python3-pip sox make

which mplayer > /dev/null
if [[ ! $? -eq 0 ]]; then
    sudo apt-get install -y mplayer
    sudo usermod -a -G audio `whoami`
    sudo usermod -a -G video `whoami`
    sudo usermod -a -G input `whoami`
    touch /tmp/driver_no_restart
    echo "add user to audio and video mod, need restart. "
fi

which g++ > /dev/null
if [[ ! $? -eq 0 ]]; then
    sudo apt-get install -y g++
fi


echo 'openai
azure-cognitiveservices-speech
evdev
httpx[socks]
edge-tts' > /tmp/requirements.txt

pip3 install -r /tmp/requirements.txt

cd display_driver && make && cd -

if [[ ! -z "$OPENAI_API_KEY" ]]; then
    echo "export OPENAI_API_KEY=your-openai-key"
fi

if [[ ! -z "$SPEECH_KEY" ]]; then
    echo "export SPEECH_KEY=your-azure-key"
fi

if [[ ! -z "$SPEECH_REGION" ]]; then
    echo "export SPEECH_REGION=your-azure-key-region"
fi