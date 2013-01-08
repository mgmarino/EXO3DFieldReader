# Makefile for linking against ROOT 
# M. Marino 22 May 2007 

WRITE_TARGETS := WriteHashFile WriteNodeFile WriteTetFile 
TARGETS :=  $(WRITE_TARGETS:%=%.1) $(WRITE_TARGETS:%=%.3) TestReader 
SOURCES = $(wildcard *.cc) #uncomment these to add all cc files in directory to your compile list 
SOURCES += $(wildcard *.C) #uncomment these to add all cc files in directory to your compile list 
OBJS = $(SOURCES:.cc=.o) 


DEBUG_FLAG := -g

TARGETOBJ = $(patsubst %, %.o, $(TARGETS))
CXX = g++

CXXFLAGS := -Wall $(DEBUG_FLAG) -I../Reader $(shell exo-config --cflags) $(shell $(CLHEP_BASE_DIR)/bin/clhep-config --include)
LIBS := $(shell $(ROOTSYS)/bin/root-config --libs --glibs) $(shell exo-config --libs) ../Reader/libReader.so 

.SECONDARY:

all: $(TARGETS)

.depend depend: 
	@echo "Checking dependencies ...."
	@g++ -M $(CXXFLAGS) $(SOURCES) > .depend

% : %.o
	@echo "Building Executable ...... $@"
	@$(CXX) -o $@ $< $(LIBS)

%.1.o : %.C 
	@echo "Compiling           ...... $@"
	@$(CXX) -c $(CXXFLAGS) -DNUM_VALUES=1 $< -o $@ 

%.3.o : %.C 
	@echo "Compiling           ...... $@"
	@$(CXX) -c $(CXXFLAGS) -DNUM_VALUES=3 $< -o $@ 

%.o: %.C 
	@echo "Compiling           ...... $@"
	@$(CXX) -c $(CXXFLAGS) $< -o $@ 

clean:
	@echo "Cleaning            ......"
	@rm -f $(TARGETS) *.o .depend

ifneq ($(MAKECMDGOALS),clean)
-include .depend
endif
