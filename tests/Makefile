BINARIES = lock_test rwlock_test
LDFLAGS = -lpthread -openmp -offload-attribute-target=mic -DRUN_MIC
BINARY_DIRECTORY = binaries/
#CXX = g++
CXX = icc -m64 -std=c++11

all: $(BINARIES)

%: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

clean:
	$(RM) *.o $(BINARIES)
