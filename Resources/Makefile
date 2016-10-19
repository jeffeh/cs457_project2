CPP_SRCS = *.cpp
CPP_OBJS = *.o
CPP_HEADERS = *.h

OBJS = ${CPP_OBJS}
EXE = chat

GPP = g++
GPP_FLAGS = -Wall -g -std=c++11 -I. -c -O2
LD_FLAGS = -Wall -I. -lX11

.cpp.o: 
	$(GPP) $(GPP_FLAGS) $(CPP_SRCS)

all: $(OBJS)
	$(GPP) $(LD_FLAGS) $(OBJS) -o $(EXE)
clean:
	rm -f *.o *~ $(EXE)
tar:
	tar -cvf PA10.tar $(CPP_SRCS) $(CPP_HEADERS) Makefile

