ROOT=$(shell pwd)
INC=$(ROOT)/inc
CC=gcc
CFLAGS=-c -fpic -Wall -Werror -m64
INCFLAGS=-I$(INC)
LIBFLAGS=-shared
OBJECTS=atomics_x86.o mcs_ticket_lock.o mcs_ticket_rwlock.o mcs_queue_lock.o mcs_queue_rwlock.o
LIBS=liblock.so

all: $(LIBS)

%.so: $(OBJECTS)
	$(CC) $(LIBFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCFLAGS) -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -o $@ $<
