TARGE=display.bin

CC=aarch64-linux-gnu-g++-10

OBJS=$(wildcard *.cpp)
FLAG=-static -g -s

%.o:%.cpp
	$(CC) -c -o $@ $^

$(TARGE):$(OBJS:.cpp=.o)
	$(CC) $(FLAG) -o $@ $^

push:
	~/ssh-dev/maixsense.sh push $(TARGE)
	echo "push done"

clean:
	rm *.o $(TARGE)

.PHONY: clean push

