ROOT=$(shell pwd)
BINARIES = lock_test rwlock_test
LDFLAGS = -lpthread -openmp -offload-attribute-target=mic -DRUN_MIC
BINARY_DIRECTORY = binaries/
#CXX = g++
CXX = icc -m64 -std=c++11
CXXFLAGS = -m64 -std=c++11
LIB = $(ROOT)/../locks
INC = $(LIB)/inc
INCFLAGS = -I$(INC)
LIBFLAGS = -L$(LIB) -llock

all: $(BINARIES)

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCFLAGS) $(LDFLAGS) $(LIBFLAGS) -o $@ $<

clean:
	$(RM) *.o $(BINARIES)
