CC=g++
CFLAGS+=-Wall -fpic -std=c++0x -I.
LDFLAGS+=-shared

ifeq ($(strip $(SIM)),)
	SIM := vcs
endif

ifeq ($(strip $(TOP)),)
	TOP := top
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

cleanall:
	for sim in cver iverilog vcs; do \
		SIM=$$sim make clean; \
	done

compileall:
	for sim in cver iverilog vcs; do \
		echo compilng $$sim ...; \
		echo; \
		SIM=$$sim make; \
		echo; \
	done
