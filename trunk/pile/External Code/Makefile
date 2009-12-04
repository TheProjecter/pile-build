# Makefile for Pile

# Where should it be installed?
PREFIX =/usr/local/share


SOURCES=goodio.cpp  NFont.cpp
HEADERS=goodio.h  NFont.h
OBJECTS=$(addsuffix .o, $(basename $(SOURCES)))

#C_SOURCES=sha1.c
#C_HEADERS=sha1.h
#C_OBJECTS=$(addsuffix .o, $(basename $(C_SOURCES)))


# Compiler (C++)
CXX=g++
CFLAGS=-Wall -O3 -ffast-math -s -fPIC
LIBS=-lstdc++

# Make sure sdl-config is available
HAVE_SDL =$(shell if (sdl-config --version) < /dev/null > /dev/null 2>&1; then echo "y"; else echo "n"; fi;)
ifeq ($(HAVE_SDL),y)
  CFLAGS += $(shell sdl-config --cflags)
endif

all:	build
	

$(OBJECTS): %.o:%.cpp  $(HEADERS)   #Each object depends on its .cpp and .h file
	$(CXX) $(CFLAGS) -c $< -o $@

$(C_OBJECTS):	%.o:%.c  $(C_HEADERS)   #Each object depends on its .c and .h file
	$(CXX) $(CFLAGS) -c $< -o $@

build: $(OBJECTS) $(C_OBJECTS)

clean:
	@rm -f *.o *.def
