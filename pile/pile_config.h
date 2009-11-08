#ifndef _PILE_CONFIG_H__
#define _PILE_CONFIG_H__

#include <list>
#include <map>
#include <string>

#include "pile_system.h"


class Configuration
{
    public:
    
    std::string exe_ext;
    std::string editor;
    
    std::map<std::string, std::string> languages;
    
    bool useSourceObjPath; // Makes the object path relative to each source file for the respective object file
    std::string objPath;
    std::list<std::string> includePaths;
    std::list<std::string> libPaths;
    
    std::string cflags;
    std::string lflags;
    std::string libraries;
    
    std::string installPath;
    
    bool useAutoDepend;
    
    Configuration()
        : exe_ext(EXE_EXT)
        , editor(DEFAULT_EDITOR)
        , useSourceObjPath(false)
        , objPath("obj/")
        , useAutoDepend(true)
    {
        languages["EDITOR"] = DEFAULT_C_COMPILER;
        languages["C_COMPILER"] = DEFAULT_C_COMPILER;
        languages["C_LINKER_D"] = DEFAULT_C_LINKER_D;
        languages["C_LINKER_S"] = DEFAULT_C_LINKER_S;
        languages["C_SYNTAX"] = DEFAULT_C_SYNTAX;
        
        languages["CPP_COMPILER"] = DEFAULT_CPP_COMPILER;
        languages["CPP_LINKER_D"] = DEFAULT_CPP_LINKER_D;
        languages["CPP_LINKER_S"] = DEFAULT_CPP_LINKER_S;
        languages["CPP_SYNTAX"] = DEFAULT_CPP_SYNTAX;
        
        languages["FORTRAN_COMPILER"] = DEFAULT_FORTRAN_COMPILER;
        languages["FORTRAN_LINKER_D"] = DEFAULT_FORTRAN_LINKER_D;
        languages["FORTRAN_LINKER_S"] = DEFAULT_FORTRAN_LINKER_S;
        languages["FORTRAN_SYNTAX"] = DEFAULT_FORTRAN_SYNTAX;
        
        languages["TARGET_PLATFORM"] = getSystemName();
        
        
        #ifdef PILE_LINUX
        includePaths.push_back("/usr/include");
        includePaths.push_back("/usr/local/include");
        #endif
        
        installPath = "/home/jonny/jon/Pile/Pile source";
    }
    
};




bool loadConfig(std::string path, Configuration& config);


#endif
