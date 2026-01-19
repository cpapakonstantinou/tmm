#Target options
TARGET = tmm
SRC = bragg.cc tmm.cc

PREFIX ?= /usr/bin
INSTALLDIR ?= $(PREFIX)

#Unit test options
TEST_TARGET = test
TEST_SRC = 
TEST_EXTRA_OBJ = 

#Directories
SRCDIR = src
INCDIR = inc
TESTDIR = test

#Toolchains
CXX = g++-13
CXX_SUFFIX = cc
LD = $(CXX)

#Compile Options
OPT=-O3
CXXFLAGS = -g -std=c++23 $(OPT) -march=native -I$(INCDIR) -I /usr/lib/damm
LDFLAGS = -L /usr/lib/damm
LDLIBS = -ldamm
TEST_LDFLAGS = 
TEST_LDLIBS =


#Shell type
SHELL := /bin/bash

#build rules
OBJ = $(SRC:%.$(CXX_SUFFIX)=$(SRCDIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.$(CXX_SUFFIX)=$(TESTDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(LD) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

$(TEST_TARGET): $(TEST_OBJ)
	$(LD) -o $@ $(TEST_OBJ) $(TEST_EXTRA_OBJ) $(TEST_LDFLAGS) $(TEST_LDLIBS)

clean:
	$(RM) $(SRCDIR)/*.o $(TESTDIR)/*.o 

cleanall: clean
	$(RM) $(TARGET) 


install: $(TARGET)
	install -m 755 $(TARGET) $(INSTALLDIR)

uninstall:
	$(RM) -r $(INSTALLDIR)/$(TARGET)

.PHONY: all clean help

.DEFAULT_GOAL := $(TARGET)
