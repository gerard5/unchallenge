#
# gnu make (host=linux,target=linux)
#

CC=gcc
CFLAGS=-Wall -O2

all: clean build

build:
	$(CC) $(CFLAGS) src/main.c -o unchallenge
	strip unchallenge

clean:
	rm -f unchallenge || true
