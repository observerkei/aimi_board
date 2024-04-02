TARGE=display.so
TEST_APP = display.app
CC=g++
ARM64_CC=aarch64-linux-gnu-g++-12

OBJS=$(wildcard *.cpp)
FLAG=
SO_FLAG=-s -g  -shared -fPIC -g $(FLAG)

all: $(TARGE)

%.o:%.cpp
	$(CC) $(SO_FLAG) -c -o $@ $^

$(TARGE):$(OBJS:.cpp=.o)
	$(CC) $(SO_FLAG) -o $@ $^

test: $(OBJS)
	mkdir -p test
	$(CC) $(FLAG) -D__DISPLAY_XTEST__ -o test/$(TEST_APP) $^
	cd test && $(MAKE)

push:
	~/ssh-dev/maixsense.sh push $(TARGE)
	echo "push done"

clean_test:
	cd test && $(MAKE) clean

clean:
	rm *.o $(TARGE)

.PHONY: clean push test

