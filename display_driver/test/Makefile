TARGE=
CC=g++
ARM64_CC=aarch64-linux-gnu-g++-12

OBJS=$(wildcard *.cpp ../*.cpp libopenai_api/libopenai_api.cpp)
FLAG= -static
SO_FLAG= -shared -fPIC -g 

all: font_bitmap.app framebuffer.app

%.o:%.cpp
	$(CC) -c -o $@ $^ $(SO_FLAG) 

font_bitmap.app:../font_bitmap.cpp
	$(CC) -D__XTEST__ -o $@ $^ $(FLAG)

framebuffer.app:../framebuffer.cpp
	$(CC) -D__XTEST__ -o $@ $^ $(FLAG)

clean:
	rm *.app

.PHONY: clean push

