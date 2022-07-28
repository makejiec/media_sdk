#
# Makefile
# makej1ec, 2022-07-10 20:20
#

TARGET := libnm.all

CUR_DIR := $(shell pwd)

CFLAGS = -Wall -g -O2 -std=c99
INC = -I $(CUR_DIR)/include

all:nm.o
	ar crv -o $(TARGET) nm.o 

nm.o:nm.c
	gcc $(CFLAGS) nm.c

clean:
	rm $(TARGET)
	rm *.o

# vim:ft=make
#
