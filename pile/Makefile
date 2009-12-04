# Makefile for Pile

# Where should it be installed?
PREFIX =/usr/local/share


SOURCES=main.cpp  pile_build.cpp  pile_commands.cpp  pile_config.cpp  pile_depend.cpp  pile_interpreter.cpp  pile_load.cpp  pile_system.cpp  pile_ui.cpp  string_functions.cpp

OBJECTS=$(addsuffix .o, $(basename $(SOURCES)))

OTHER_OBJECTS="External Code/goodio.o" "External Code/NFont.o" "Eve Source/eve_builtInFunctions.o" "Eve Source/eve_evaluater.o" "Eve Source/eve_functions.o" "Eve Source/eve_interpreter.o" "Eve Source/eve_operators.o" "Eve Source/eve_tokenizer.o" "Eve Source/eve_variables.o"

HEADERS=pile_build.h  pile_commands.h  pile_config.h  pile_depend.h  pile_env.h  pile_global.h  pile_load.h  pile_os.h  pile_system.h  pile_ui.h  string_functions.h

# Compiler (C++)
CXX=g++
CFLAGS=-Wall -O3 -ffast-math -s -fPIC
LFLAGS='-Wl,-rpath,$$ORIGIN'
LIBS=-lstdc++

# Make sure sdl-config is available
HAVE_SDL =$(shell if (sdl-config --version) < /dev/null > /dev/null 2>&1; then echo "y"; else echo "n"; fi;)
ifeq ($(HAVE_SDL),n)
  CFLAGS += -DPILE_NO_GUI
else
  CFLAGS += $(shell sdl-config --cflags)
  LIBS += $(shell sdl-config --libs) -lSDL_ttf
endif


all:	build ready_to_install
	

$(OBJECTS): %.o:%.cpp   #Each object depends on its .cpp and .h file
	$(CXX) $(CFLAGS) -c $< -o $@

$(C_OBJECTS):	%.o:%.c  $(HEADERS)   #Each object depends on its .c and .h file
	$(CXX) $(CFLAGS) -c $<

build: $(OBJECTS) other_dirs
	$(CXX) $(LFLAGS) -o pile $(OBJECTS) $(OTHER_OBJECTS) $(LIBS)
	

other_dirs: 
	(cd Eve\ Source; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all);
	(cd External\ Code; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all);

ready_to_install: 
	@echo ~~ Ready!  Type \'make install\' to install to $(PREFIX)/pile ~~

pile: build
	

clean:
	@rm -f *.o *.def

install:
	mkdir -p $(PREFIX)/pile
	@#install -d -m 655 Documentation $(PREFIX)/pile
	@#install -d -m 655 fonts $(PREFIX)/pile
	cp -r Documentation $(PREFIX)/pile
	chmod 655 $(PREFIX)/pile/Documentation
	cp -r fonts $(PREFIX)/pile
	chmod 655 $(PREFIX)/pile/fonts
	install -m 644 pile $(PREFIX)/pile/
	chmod a+x $(PREFIX)/pile/pile
	install -m 644 template_pile_linux.conf $(PREFIX)/pile/template_pile.conf

	mkdir -p ~/.pile
	install -m 664 template_pile_linux.conf ~/.pile/template_pile.conf
	
	rm /usr/bin/pile
	ln $(PREFIX)/pile/pile /usr/bin/pile
	@echo
	@echo "** Pile installed in $(PREFIX)/pile"
