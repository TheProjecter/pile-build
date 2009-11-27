/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_env.h

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the Environment class definition.
*/

#ifndef _PILE_ENV_H__
#define _PILE_ENV_H__

#include <string>

#include "pile_ui.h"
#include "pile_depend.h"
#include "pile_config.h"
#include "Eve Source/eve_interpreter.h"

Variable* fn_scan(Variable* arg1, Variable* arg2);
Variable* fn_build(Variable* arg1, Variable* arg2, Variable* arg3);
Variable* fn_link(Variable* arg1, Variable* arg2, Variable* arg3, Variable* arg4, Variable* arg5);

Variable* createNewVariableFromCommandLine(const std::string& name, const std::string& value);
void changeVariableFromCommandLine(Variable* var, const std::string& name, const std::string& value);

class Environment;
extern Environment env;

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
    std::list<std::string> variants;
    
    std::map<std::string, std::string> variables;
    
    bool dryRun;
    bool noCompile;
    bool noLink;
    
    std::string pilefile;
    
    Environment()
        : outfile("a.out")
        , dryRun(false)
        , noCompile(false)
        , noLink(false)
    {}
    
    void loadConfig(Configuration& config)
    {
        outfile = ("a.out" + config.exe_ext);
    }
    
    void initInterpreter(Interpreter& inter, Configuration& config)
    {
        // Add builtin variables
        Scope& s = *(inter.env.begin());
        
        // VARIABLES
        s.env["PERMIT"] = new String("PERMIT", "none");
        s.env["PERMIT"]->reference = true;
        s.env["OUTPUT"] = new String("OUTPUT", outfile);
        s.env["OUTPUT"]->reference = true;
        
        s.env["SOURCES"] = new Array("SOURCES", STRING);
        s.env["SOURCES"]->reference = true;
        s.env["CFLAGS"] = new Array("CFLAGS", STRING);
        s.env["CFLAGS"]->reference = true;
        
        s.env["LFLAGS"] = new Array("LFLAGS", STRING);
        s.env["LFLAGS"]->reference = true;
        s.env["OBJECTS"] = new Array("OBJECTS", STRING);
        s.env["OBJECTS"]->reference = true;
        s.env["LIBRARIES"] = new Array("LIBRARIES", STRING);
        s.env["LIBRARIES"]->reference = true;
        s.env["HOST_PLATFORM"] = new String("HOST_PLATFORM", getSystemName());
        s.env["TARGET_PLATFORM"] = new String("TARGET_PLATFORM", config.languages["TARGET_PLATFORM"]);
        
        
        Array* vars = new Array("VARIANTS", STRING);
        s.env["VARIANTS"] = vars;
        
        Array* opts = new Array("OPTIONS", STRING);
        s.env["OPTIONS"] = opts;
        // includeDirs
        // libDirs
        
        // CLASSES
        
        // Compiler
        Class* compiler = new Class("Compiler");
        compiler->addVariable("string", "name");
        compiler->addVariable("string", "path");
        Function* compile = new Function("compile", &fn_build);
        compiler->addFunction("compile", compile);
        Function* scan = new Function("scan", &fn_scan);
        compiler->addFunction("scan", scan);
        //s.env["Compiler"] = compiler;
        inter.addClass(compiler);
        ClassObject* cpp_compiler = new ClassObject("cpp_compiler", "Compiler");
        Variable* cpp_name = cpp_compiler->getVariable("name");
        Variable* cpp_path = cpp_compiler->getVariable("path");
        if(cpp_name != NULL && cpp_path != NULL && cpp_name->getType() == STRING && cpp_path->getType() == STRING)
        {
            static_cast<String*>(cpp_name)->setValue(config.languages["CPP_COMPILER"]);
            static_cast<String*>(cpp_path)->setValue(config.languages["CPP_COMPILER"]);
        }
        s.env["cpp_compiler"] = cpp_compiler;
        
        // Linker
        Class* linker = new Class("Linker");
        linker->addVariable("string", "name");
        linker->addVariable("string", "path");
        Function* linkit = new Function("link", &fn_link);
        linker->addFunction("link", linkit);
        //s.env["Compiler"] = compiler;
        inter.addClass(linker);
        ClassObject* cpp_linker = new ClassObject("cpp_linker", "Linker");
        Variable* cpp_linkname = cpp_linker->getVariable("name");
        Variable* cpp_linkpath = cpp_linker->getVariable("path");
        if(cpp_linkname != NULL && cpp_linkpath != NULL && cpp_linkname->getType() == STRING && cpp_linkpath->getType() == STRING)
        {
            static_cast<String*>(cpp_linkname)->setValue(config.languages["CPP_LINKER_D"]);
            static_cast<String*>(cpp_linkpath)->setValue(config.languages["CPP_LINKER_D"]);
        }
        s.env["cpp_linker"] = cpp_linker;
        
        
        // Init command line variables
        map<string, string>::iterator fl;
        
        // Need to check the other built-in variables!
        for(fl = env.variables.begin(); fl != env.variables.end(); fl++)
        {
            map<string, Variable*>::iterator v = s.env.find(fl->first);
            if(v != s.env.end())
            {
                changeVariableFromCommandLine(v->second, fl->first, fl->second);
            }
            else
            {
                // Add new variable
                Variable* v1 = createNewVariableFromCommandLine(fl->first, fl->second);
                if(v1 != NULL)
                {
                    s.env[fl->first] = v1;
                    s.env[fl->first]->reference = true;
                }
            }
        }
        
        /*fl = env.variables.find("VARIANTS");
        if(fl != env.variables.end())
        {
            list<string> l = ioExplode(fl->second, ',');
            for(list<string>::iterator e = l.begin(); e != l.end(); e++)
            {
                if(*e != "")
                    variants.push_back(*e);
            }
            env.variables.erase(fl);
        }*/
        
        if(variants.size() == 0 && vars->size() == 0)
            vars->push_back(new String("<temp>", "default"));
        for(std::list<std::string>::iterator e = variants.begin(); e != variants.end(); e++)
        {
            vars->push_back(new String("<temp>", *e));
        }
        
        
        if(dryRun)
            opts->push_back(new String("<temp>", "dry_run"));
        if(noCompile)
            opts->push_back(new String("<temp>", "no_compile"));
        if(noLink)
            opts->push_back(new String("<temp>", "no_link"));
        
        /*fl = env.variables.find("CFLAGS");
        if(fl != env.variables.end())
        {
            list<string> l = ioExplode(fl->second, ',');
            for(list<string>::iterator e = l.begin(); e != l.end(); e++)
            {
                if(*e != "")
                    static_cast<Array*>(s.env["CFLAGS"])->push_back(new String("<temp>", *e));
            }
            env.variables.erase(fl);
        }*/
        
        for(list<string>::iterator e = cflags.begin(); e != cflags.end(); e++)
        {
            static_cast<Array*>(s.env["CFLAGS"])->push_back(new String("<temp>", *e));
        }
        
        /*fl = env.variables.find("LFLAGS");
        if(fl != env.variables.end())
        {
            list<string> l = ioExplode(fl->second, ',');
            for(list<string>::iterator e = l.begin(); e != l.end(); e++)
            {
                if(*e != "")
                    static_cast<Array*>(s.env["LFLAGS"])->push_back(new String("<temp>", *e));
            }
            env.variables.erase(fl);
        }*/
        
        for(list<string>::iterator e = lflags.begin(); e != lflags.end(); e++)
        {
            static_cast<Array*>(s.env["LFLAGS"])->push_back(new String("<temp>", *e));
        }
        
        
        
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
