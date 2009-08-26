#ifndef _PILE_SYSTEM_H__
#define _PILE_SYSTEM_H__

#include <string>

#include "pile_os.h"

#ifdef __WIN32__
    #define PILE_WIN32
#else
    #define PILE_LINUX
#endif

#ifdef PILE_WIN32
    #define EXE_EXT ".exe"
    #define DEFAULT_EDITOR "c:/windows/notepad.exe"
    #define DEFAULT_C_COMPILER ""
    #define DEFAULT_C_LINKER_D ""
    #define DEFAULT_C_LINKER_S ""
    #define DEFAULT_C_SYNTAX "COMP -c FLAGS SRC"
    #define DEFAULT_CPP_COMPILER "\"C:/Program Files/CodeBlocks/MinGW/bin/mingw32-g++.exe\""
    #define DEFAULT_CPP_LINKER_D "\"C:/Program Files/CodeBlocks/MinGW/bin/mingw32-g++.exe\""
    #define DEFAULT_CPP_LINKER_S ""
    #define DEFAULT_CPP_SYNTAX "COMP -c FLAGS SRC"
    #define DEFAULT_FORTRAN_COMPILER ""
    #define DEFAULT_FORTRAN_LINKER_D ""
    #define DEFAULT_FORTRAN_LINKER_S ""
    #define DEFAULT_FORTRAN_SYNTAX "COMP -c FLAGS SRC"
#endif

#ifdef PILE_LINUX
    #define EXE_EXT ""
    #define DEFAULT_EDITOR "vi"
    #define DEFAULT_C_COMPILER "gcc"
    #define DEFAULT_C_LINKER_D "gcc"
    #define DEFAULT_C_LINKER_S "ar rsc"
    #define DEFAULT_C_SYNTAX "COMP -c FLAGS SRC"
    #define DEFAULT_CPP_COMPILER "g++"
    #define DEFAULT_CPP_LINKER_D "g++"
    #define DEFAULT_CPP_LINKER_S "ar rsc"
    #define DEFAULT_CPP_SYNTAX "COMP -c FLAGS SRC"
    #define DEFAULT_FORTRAN_COMPILER "gfortran"
    #define DEFAULT_FORTRAN_LINKER_D "gfortran"
    #define DEFAULT_FORTRAN_LINKER_S "ar rsc"
    #define DEFAULT_FORTRAN_SYNTAX "COMP -c FLAGS SRC"
#endif



std::string getHomeDir();
inline std::string getConfigDir()
{
    return getHomeDir() + "/.pile";
}

void convertSlashes(std::string& str);

void systemCall(std::string command);

void delay(unsigned int milliseconds);

#endif
