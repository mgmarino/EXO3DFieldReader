# Makefile for linking against ROOT 
# M. Marino 22 May 2007 


SOSUFFIX := so 
SOFLAGS  := -shared -Wl
NAME := $(notdir $(shell pwd))

SHLIB :=  lib$(NAME).$(SOSUFFIX)
SOURCES = $(wildcard *.cc) #uncomment these to add all cc files in directory to your compile list 
OBJECTS = $(SOURCES:.cc=.o) 


DEBUG_FLAG := -g -O2

CXX = g++

CXXFLAGS := -Wall $(DEBUG_FLAG) -fPIC -I. $(shell exo-config --cflags) $(shell $(CLHEP_BASE_DIR)/bin/clhep-config --include)
LIBS := $(shell $(ROOTSYS)/bin/root-config --libs --glibs) $(shell exo-config --libs) 

ROOTDICTHEADERS := $(shell grep -l ClassDef $(wildcard *.h) | xargs) 
ROOTDICTCCNAME  := $(NAME)DICT.C
ROOTDICTHHNAME  := $(NAME)DICT.h
ROOTDICTOBJECT  := $(ROOTDICTCCNAME:.C=.o)
ROOTCINT        := rootcint

.SECONDARY:

all: $(SHLIB)

.depend depend: 
	@echo "Checking dependencies ...."
	@g++ -M $(CXXFLAGS) $(SOURCES) > .depend

%.o: %.C 
	@echo "Building .... $@"
	@$(CXX) -c $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $< 

%.o: %.cc 
	@echo "Building .... $@"
	@$(CXX) -c $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $< 

% : %.o
	@echo "Building Executable ...... $@"
	@$(CXX) -o $@ $< $(LIBS)

$(ROOTDICTCCNAME): $(ROOTDICTHEADERS) LinkDef.h
	@echo Rootifying files...
	@rm -f $(ROOTDICTCCNAME) $(ROOTDICTHHNAME) 
	@$(ROOTCINT) $(ROOTDICTCCNAME) -c -p $(CXXFLAGS) $(ROOTDICTHEADERS) LinkDef.h
 
$(SHLIB): $(OBJECTS) $(ROOTDICTOBJECT)
	@echo "Building so .... $@"
	@$(CXX) $(SOFLAGS) -o $@ $^ $(LIBS)


clean:
	@echo "Cleaning            ......"
	@rm -f $(ROOTDICTHHNAME) $(ROOTDICTCCNAME) *.o .depend
	@rm -f $(SHLIB)

ifneq ($(MAKECMDGOALS),clean)
-include .depend
endif
