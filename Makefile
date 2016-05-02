ROOT=$(shell pwd)
INC=$(ROOT)/locks/inc
CC=icc
CPPFLAGS=-std=c++11 -fopenmp -fpic -m64 #-mmic
CFLAGS=-c -fpic -Wall -m64 -fopenmp #-mmic
INCFLAGS=-I$(INC)
LDFLAGS=
#LIBFLAGS=-static
LOCKS=locks/atomics_x86.o locks/mcs_ticket_lock.o locks/mcs_ticket_rwlock.o\
	locks/mcs_queue_lock.o locks/mcs_queue_rwlock.o
TESTS=tests/lock_test tests/rwlock_test

all: $(LOCKS) $(TESTS)

%: %.cpp
	$(CC) $(CPPFLAGS) $(INCFLAGS) $(LOCKS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCFLAGS) -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCFLAGS) -o $@ $<

clean:
	rm -rf $(LOCKS) $(TESTS)
