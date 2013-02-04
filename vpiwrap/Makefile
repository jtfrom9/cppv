CC=g++
CFLAGS=-Wall -fpic -std=c++0x
LDFLAGS=-shared

ifeq ($(strip $(SIM)),)
	SIM := vcs
endif

all: compile

include Makefile.$(SIM)

OBJS = vpitest.o

.cpp.o:
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

compile: $(TARGET)

clean: clean_sim
	rm -rf *~ *.o $(OBJS) $(TARGET)
