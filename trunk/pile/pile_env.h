#ifndef _PILE_ENV_H__
#define _PILE_ENV_H__

#include <string>
#include "pile_depend.h"
#include "pile_config.h"
#include "Eve Source/eve_interpreter.h"

class Environment
{
    public:
    std::string outfile;
    
    std::list<std::string> sources;
    std::list<std::string> objects;
    std::map<FileData*, std::list<FileData*> > depends;
    std::map<std::string, FileData*> fileDataHash;
    std::list<std::string> cflags;
    std::list<std::string> lflags;
    
    std::string pilefile;
    
    Environment()
    {}
    
    void loadConfig(Configuration& config)
    {
        outfile = ("a.out" + config.exe_ext);
    }
    
    void initInterpreter(Interpreter& inter)
    {
        // Add builtin variables
        (*(inter.env.begin())).env["print"] = new Function(FN_PRINT);
        (*(inter.env.begin())).env["type"] = new Function(FN_TYPE);
        (*(inter.env.begin())).env["string"] = new Function(FN_STRING);
        (*(inter.env.begin())).env["int"] = new Function(FN_INT);
        (*(inter.env.begin())).env["float"] = new Function(FN_FLOAT);
        (*(inter.env.begin())).env["output"] = new String("a.out");
        (*(inter.env.begin())).env["sources"] = new Array(STRING);
        (*(inter.env.begin())).env["cflags"] = new Array(STRING);
        (*(inter.env.begin())).env["lflags"] = new Array(STRING);
        // includeDirs
        // libDirs
    }
    
    void finalizeInterpreter(Interpreter& inter)
    {
        Variable* v;
        
        v = (*(inter.env.begin())).env["output"];
        if(v != NULL && v->getType() == STRING)
        {
            String* s = static_cast<String*>(v);
            outfile = s->getValue();
        }
        
        v = (*(inter.env.begin())).env["sources"];
        if(v != NULL && v->getType() == ARRAY)
        {
            Array* s = static_cast<Array*>(v);
            if(s->getValueType() == STRING)
            {
                for(std::vector<Variable*>::iterator e = s->getValue().begin(); e != s->getValue().end(); e++)
                {
                    sources.push_back((*e)->getValueString());
                }
            }
        }
        
        v = (*(inter.env.begin())).env["cflags"];
        if(v != NULL && v->getType() == ARRAY)
        {
            Array* s = static_cast<Array*>(v);
            if(s->getValueType() == STRING)
            {
                for(std::vector<Variable*>::iterator e = s->getValue().begin(); e != s->getValue().end(); e++)
                {
                    cflags.push_back((*e)->getValueString());
                }
            }
        }
        
        v = (*(inter.env.begin())).env["lflags"];
        if(v != NULL && v->getType() == ARRAY)
        {
            Array* s = static_cast<Array*>(v);
            if(s->getValueType() == STRING)
            {
                for(std::vector<Variable*>::iterator e = s->getValue().begin(); e != s->getValue().end(); e++)
                {
                    lflags.push_back((*e)->getValueString());
                }
            }
        }
    }
    
    void print()
    {
        UI_debug_pile("Printing environment...\n");
        std::list<std::string>* ls;
        
        UI_debug_pile(" outfile:\n");
        UI_debug_pile("  %s\n", outfile.c_str());
        
        ls = &sources;
        UI_debug_pile(" sources:\n");
        for(std::list<std::string>::iterator e = ls->begin(); e != ls->end(); e++)
        {
            UI_debug_pile("  %s\n", e->c_str());
        }
        
        ls = &objects;
        UI_debug_pile(" objects:\n");
        for(std::list<std::string>::iterator e = ls->begin(); e != ls->end(); e++)
        {
            UI_debug_pile("  %s\n", e->c_str());
        }
        
        ls = &cflags;
        UI_debug_pile(" cflags:\n");
        for(std::list<std::string>::iterator e = ls->begin(); e != ls->end(); e++)
        {
            UI_debug_pile("  %s\n", e->c_str());
        }
        
        ls = &lflags;
        UI_debug_pile(" lflags:\n");
        for(std::list<std::string>::iterator e = ls->begin(); e != ls->end(); e++)
        {
            UI_debug_pile("  %s\n", e->c_str());
        }
    }
};



#endif