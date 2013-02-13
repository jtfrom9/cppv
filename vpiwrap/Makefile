CC=g++
CFLAGS+=-Wall -fpic -std=c++0x -I.
LDFLAGS+=-shared
DEPFILE=makefile.dep

ifeq ($(strip $(SIM)),)
	SIM := vcs
endif

ifeq ($(strip $(TOP)),)
	TOP := top
endif

all: dep compile

include Makefile.$(SIM)

OBJS = vpitest.o startup.o objects.o Simulator.o Process.o ProcessManager.o Value.o

.cpp.o: 
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

compile: $(TARGET)

clean: clean_sim
	rm -rf *~ *.o $(OBJS) $(TARGET) $(DEPFILE)

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

dep: 
	g++ -MM $(CFLAGS) $(OBJS:.o=.cpp)  | sed -b -e 's/[a-zA-Z0-9_-]*\.cpp//g' > $(DEPFILE)

-include $(DEPFILE)
