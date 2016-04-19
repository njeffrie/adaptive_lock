BINARIES = ticket_lock_basic
LDFLAGS = -lpthread
BINARY_DIRECTORY = binaries/
CXX = g++

all: $(BINARIES)
%.o: %.cu
	$(NVCC) $(ICUDAFLAGS) -c -o $@ $<

%: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

clean:
	$(RM) *.o $(BINARIES)
