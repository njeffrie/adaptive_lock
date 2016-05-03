ROOT=$(shell pwd)
INC=$(ROOT)/locks/inc
LOCKDIR=$(ROOT)/locks
TESTDIR=$(ROOT)/tests

CXX=icc -m64 #-std=c++11
CXXFLAGS=-I$(INC) -O3 -Wall -fopenmp -offload-attribute-target=mic -DRUN_MIC

#CPPFLAGS=-std=c++11 -fpic -m64 -O3 -Wall -openmp -offload-attribute-target=mic -DRUN_MIC
#CFLAGS=-c -fpic -Wall -m64 -O3 -openmp -offload-attribute-target=mic -DRUN_MIC
#INCFLAGS=-I$(INC)


OBJS=$(LOCKDIR)/atomics_x86.o $(LOCKDIR)/mcs_ticket_lock.o $(LOCKDIR)/mcs_ticket_rwlock.o\
	$(LOCKDIR)/mcs_queue_lock.o $(LOCKDIR)/mcs_queue_rwlock.o #$(TESTDIR)/lock_test.o\
#$(TESTDIR)/rwlock_test.o

TESTS=tests/lock_test tests/rwlock_test

all: clean $(OBJS) $(TESTS)

#%: $(OBJS)
#	$(CXX) $(CXXFLAGS) -std=c++11 -o $@ $(OBJS)

%: %.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 $(OBJS) $< -o $@

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.S
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJS) $(TESTS)
