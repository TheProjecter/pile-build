# Makefile for Pile

# Where should it be installed?
PREFIX =/usr/local/share


SOURCES=eve_builtInFunctions.cpp  eve_functions.cpp  eve_tokenizer.cpp  eve_evaluater.cpp  eve_interpreter.cpp  eve_operators.cpp  eve_variables.cpp

OBJECTS=$(addsuffix .o, $(basename $(SOURCES)))

HEADERS=eve_interpreter.h

# Compiler (C++)
CXX=g++
CFLAGS=-Wall -O3 -ffast-math -s -fPIC 
LIBS=-lstdc++


all:	build
	

$(OBJECTS): %.o:%.cpp   #Each object depends on its .cpp and .h file
	$(CXX) $(CFLAGS) -c $< -o $@

$(C_OBJECTS):	%.o:%.c  $(HEADERS)   #Each object depends on its .c and .h file
	$(CXX) $(CFLAGS) -c $<

build: $(OBJECTS)
	

clean:
	@rm -f *.o *.def
