#ifndef _PILE_ENV_H__
#define _PILE_ENV_H__

#include <string>
#include "pile_depend.h"
#include "pile_config.h"
#include "Eve Source/eve_interpreter.h"

Variable* fn_build(Variable* arg1, Variable* arg2, Variable* arg3);
Variable* fn_link(Variable* arg1, Variable* arg2, Variable* arg3, Variable* arg4, Variable* arg5);

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
    
    bool dryRun;
    bool noCompile;
    bool noLink;
    
    std::string pilefile;
    
    Environment()
        : dryRun(false)
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
        s.env["OUTPUT"] = new String("a.out");
        s.env["OUTPUT"]->reference = true;
        s.env["SOURCES"] = new Array(STRING);
        s.env["SOURCES"]->reference = true;
        s.env["CFLAGS"] = new Array(STRING);
        s.env["CFLAGS"]->reference = true;
        s.env["LFLAGS"] = new Array(STRING);
        s.env["LFLAGS"]->reference = true;
        s.env["OBJECTS"] = new Array(STRING);
        s.env["OBJECTS"]->reference = true;
        s.env["LIBRARIES"] = new Array(STRING);
        s.env["LIBRARIES"]->reference = true;
        s.env["HOST_PLATFORM"] = new String(getSystemName());
        s.env["TARGET_PLATFORM"] = new String(config.languages["TARGET_PLATFORM"]);
        //s.env["CPP_COMPILER"] = new Compiler(config.languages["CPP_COMPILER"]);
        
        // Compiler
        Class* compiler = new Class("Compiler");
        compiler->addVariable("string", "name");
        compiler->addVariable("string", "path");
        Function* compile = new Function(&fn_build);
        compiler->addFunction("compile", compile);
        //s.env["Compiler"] = compiler;
        inter.addClass(compiler);
        ClassObject* cpp_compiler = new ClassObject("Compiler");
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
        Function* linkit = new Function(&fn_link);
        linker->addFunction("link", linkit);
        //s.env["Compiler"] = compiler;
        inter.addClass(linker);
        ClassObject* cpp_linker = new ClassObject("Linker");
        Variable* cpp_linkname = cpp_linker->getVariable("name");
        Variable* cpp_linkpath = cpp_linker->getVariable("path");
        if(cpp_linkname != NULL && cpp_linkpath != NULL && cpp_linkname->getType() == STRING && cpp_linkpath->getType() == STRING)
        {
            static_cast<String*>(cpp_linkname)->setValue(config.languages["CPP_LINKER_D"]);
            static_cast<String*>(cpp_linkpath)->setValue(config.languages["CPP_LINKER_D"]);
        }
        s.env["cpp_linker"] = cpp_linker;
        
        Array* vars = new Array(STRING);
        if(variants.size() == 0)
            vars->push_back(new String("default"));
        for(std::list<std::string>::iterator e = variants.begin(); e != variants.end(); e++)
        {
            vars->push_back(new String(*e));
        }
        s.env["VARIANTS"] = vars;
        
        Array* opts = new Array(STRING);
        if(dryRun)
            opts->push_back(new String("dry_run"));
        if(noCompile)
            opts->push_back(new String("no_compile"));
        if(noLink)
            opts->push_back(new String("no_link"));
        s.env["OPTIONS"] = opts;
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
