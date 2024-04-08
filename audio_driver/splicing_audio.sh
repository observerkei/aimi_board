#!/bin/bash


which sox > /dev/null
if [[ ! $? -eq 0 ]]; then
    sudo apt-get install -y sox
fi

sox $@