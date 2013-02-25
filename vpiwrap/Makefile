ifeq ($(strip $(SIM)),)
	SIM := vcs
endif

ifeq ($(strip $(TOP)),)
	TOP := top
endif

ifeq ($(strip $(BASEDIR)),)
	BASEDIR := .
endif

VPATH = $(BASEDIR)

CC			= g++
CFLAGS     += -Wall -fPIC -std=c++0x -I$(BASEDIR)
LDFLAGS	   += -shared
DEPFILE		= makefile.dep
INITCALLBACK=initialCallback

all: dep build

include $(BASEDIR)/Makefile.$(SIM)

VPICPP_OBJS = startup.o Simulator.o Process.o ProcessManager.o Value.o ObjectImpl.o Signal.o

.cpp.o: 
	$(CC) $(CFLAGS) -c $<

$(TESTBENCH_LIB): $(VPICPP_OBJS) $(TESTBENCH_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(EXTRA_LDFLAGS)

lib: $(TESTBENCH_LIB) 

run: build
	$(RUN_COMMAND) $(SIM_ARGS)

debug: build
	gdb -tui --arg $(RUN_COMMAND)

clean: cleansim
	rm -rf *~ *.o $(VPICPP_OBJS) $(TESTBENCH_OBJS) $(TESTBENCH_LIB) $(DEPFILE)

cleanall:
	for sim in cver iverilog vcs; do \
		SIM=$$sim make clean; \
	done

buildliball:
	for sim in cver iverilog vcs; do \
		echo compilng $$sim ...; \
		echo; \
		SIM=$$sim make; \
		echo; \
	done

dep: 
	g++ -MM $(CFLAGS) \
	$(addprefix $(BASEDIR)/,$(VPICPP_OBJS:.o=.cpp)) \
	$(TESTBENCH_OBJS:.o=.cpp) \
	| sed -b -e 's/[a-zA-Z0-9_-]*\.cpp//g' > $(DEPFILE)
-include $(DEPFILE)

