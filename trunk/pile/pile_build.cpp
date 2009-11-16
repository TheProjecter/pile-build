#include "pile_global.h"
#include "pile_config.h"
#include "pile_depend.h"
#include "pile_env.h"
#include "pile_commands.h"
#include "string_functions.h"

bool isCExt(const string& ext);
bool isCPPExt(const string& ext);
bool isFORTRANExt(const string& ext);
string getBaseName(string file);

extern Environment env;
extern Configuration config;
extern Interpreter interpreter;

/*
Gets the file extension of a given file.

Takes: string (file name)
Returns: string (file extension)
*/
string getExtension(const string& file)
{
    unsigned int dotpos = file.find_last_of(".");
    if(dotpos != string::npos)
    {
        string ext = file.substr(dotpos, string::npos);
        toLower(ext);
        return ext;
    }
    return "";
}

// LANGUAGE
string getCompiler(Configuration& config, const string& file)
{
    string ext = getExtension(file);
    if(ext != "")
    {
        if(isCExt(ext))
            return config.languages["C_COMPILER"];
        if(isCPPExt(ext))
            return config.languages["CPP_COMPILER"];
        if(isFORTRANExt(ext))
            return config.languages["FORTRAN_COMPILER"];
        //if(isJavaExt(ext))
        //    return config["JAVA_COMPILER"];
    }
    return "";
}



// Returns VOID (NULL)
// Takes Compiler, array<string> sources
Variable* fn_scan(Variable* arg1, Variable* arg2)
{
    ClassObject* c = dynamic_cast<ClassObject*>(arg1);
    Array* sources = dynamic_cast<Array*>(arg2);
    
    if(c == NULL || sources == NULL)
        return NULL;
    
    string path = static_cast<String*>(c->getVariable("path"))->getValue();
    vector<Variable*> sourceFiles = sources->getValue();
    
    string sourceFile;
    for(vector<Variable*>::iterator e = sourceFiles.begin(); e != sourceFiles.end(); e++)
    {
        if((*e)->getType() != STRING)
        {
            interpreter.error("Wrong type in array sent to build().\n");
            return NULL;
        }
        String* s = static_cast<String*>(*e);
        sourceFile = s->getValue();
        
        recurseIncludes(env.depends, env.fileDataHash, config.includePaths, sourceFile, "");
    }
    
    return NULL;
}

// Returns array<string> objectFiles
Variable* fn_build(Variable* arg1, Variable* arg2, Variable* arg3)
{
    ClassObject* c = dynamic_cast<ClassObject*>(arg1);
    Array* sources = dynamic_cast<Array*>(arg2);
    Array* opts = dynamic_cast<Array*>(arg3);
    
    if(c == NULL || sources == NULL || opts == NULL)
        return NULL;
    
    string path = static_cast<String*>(c->getVariable("path"))->getValue();
    vector<Variable*> sourceFiles = sources->getValue();
    
    string options;
    for(vector<Variable*>::iterator e = opts->getValue().begin(); e != opts->getValue().end(); e++)
    {
        if((*e)->getType() != STRING)
        {
            interpreter.error("Wrong type in array sent to build().\n");
            return NULL;
        }
        String* s = static_cast<String*>(*e);
        options += s->getValue() + " ";
    }
    
    Array* resultObjects = new Array("<temp>", STRING);
    
    
    char buffer[5000];
    string objName;
    string sourceFile, sourceFileQuoted;
    string tempname = ".pile.tmp";
    list<string> failedFiles;
    ioDelete(tempname.c_str());
    
    UI_debug_pile("Checking sources for building.\n");
    //UI_debug_pile("Sources size: %d\n", env.sources.size());
    for(vector<Variable*>::iterator e = sourceFiles.begin(); e != sourceFiles.end(); e++)
    {
        if((*e)->getType() != STRING)
        {
            interpreter.error("Wrong type in array sent to build().\n");
            return NULL;
        }
        String* s = static_cast<String*>(*e);
        sourceFile = s->getValue();
        
        
        if(!ioExists(sourceFile))
        {
            UI_error("Source file \"%s\" not found.\n", sourceFile.c_str());
            continue;
        }
        sourceFileQuoted = quoteWhitespace(sourceFile);
        //sourceFile = quoteWhitespace(*e);
        FileData* fd = env.fileDataHash[sourceFile];
        objName = getObjectName(sourceFile, config.objPath, config.useSourceObjPath);
        mkpath(ioStripToDir(objName));
        objName = quoteWhitespace(objName);
        //objName = quoteWhitespace(sourceFile + ".o");
        
        
        // FIXME: mustRebuild() is crashing... Is it fixed yet?
        if(mustRebuild(objName, env.depends, fd))
        {
            sprintf(buffer, "%s %s -c %s -o %s", path.c_str(), options.c_str(), sourceFileQuoted.c_str(), objName.c_str());
            
            
            string buff = buffer;
            convertSlashes(buff);
            
            UI_print(" Building %s\n  %s\n", sourceFile.c_str(), buff.c_str());
            
            
            UI_debug_pile("Actual call:\n %s\n", buff.c_str());
            
            // Needed for success check
            bool objExists = ioExists(objName);
            time_t objTime = 0;
            if(objExists)
                objTime = ioTimeModified(objName);
            
            systemCall(buff.c_str());
            
            UI_print_file(tempname);
            ioDelete(tempname.c_str());
            
            // Check to see if the build was successful.
            if(objExists)
            {
                // FIXME: This looks like it could fail if the build is quick (and the object had been modified...)!!!
                if(!ioExists(objName) || objTime >= ioTimeModified(objName))
                    failedFiles.push_back(sourceFile);
                    
            }
            else
            {
                if(!ioExists(objName))
                    failedFiles.push_back(sourceFile);
            }
        }
        else
        {
            UI_print(" Up to date: %s\n", sourceFile.c_str());
        }
        
        resultObjects->push_back(new String("<temp>", objName));
        
        if(UI_processEvents() < 0)
            return NULL;
        UI_updateScreen();
    }
    
    if(failedFiles.size() > 0)
    {
        UI_error("Some files failed to build:\n");
        for(list<string>::iterator e = failedFiles.begin(); e != failedFiles.end(); e++)
        {
            UI_error("  %s\n", e->c_str());
        }
        UI_error("\n");
    }
    
    return resultObjects;
}


// Returns VOID (NULL)
// Params: ClassObject linker, string outfile, array objects, array options, array libraries
Variable* fn_link(Variable* arg1, Variable* arg2, Variable* arg3, Variable* arg4, Variable* arg5)
{
    ClassObject* c = dynamic_cast<ClassObject*>(arg1);
    String* outname = dynamic_cast<String*>(arg2);
    Array* objs = dynamic_cast<Array*>(arg3);
    Array* opts = dynamic_cast<Array*>(arg4);
    Array* libs = dynamic_cast<Array*>(arg5);
    
    if(c == NULL || outname == NULL || objs == NULL || opts == NULL)
        return NULL;
    
    string path = static_cast<String*>(c->getVariable("path"))->getValue();
    vector<Variable*> objects = objs->getValue();
    
    string options;
    for(vector<Variable*>::iterator e = opts->getValue().begin(); e != opts->getValue().end(); e++)
    {
        if((*e)->getType() != STRING)
        {
            interpreter.error("Wrong type in array sent to build().\n");
            return NULL;
        }
        String* s = static_cast<String*>(*e);
        options += s->getValue() + " ";
    }
    
    string libraries;
    for(vector<Variable*>::iterator e = libs->getValue().begin(); e != libs->getValue().end(); e++)
    {
        if((*e)->getType() != STRING)
        {
            interpreter.error("Wrong type in array sent to build().\n");
            return NULL;
        }
        String* s = static_cast<String*>(*e);
        libraries += s->getValue() + " ";
    }
    
    string objectstr;
    for(vector<Variable*>::iterator e = objects.begin(); e != objects.end(); e++)
    {
        if((*e)->getType() != STRING)
        {
            interpreter.error("Wrong type in array sent to build().\n");
            return NULL;
        }
        String* s = static_cast<String*>(*e);
        objectstr += quoteWhitespace(s->getValue()) + " ";
    }
    
    
    char buffer[5000];
    string out = quoteWhitespace(outname->getValue() + EXE_EXT);
    string tempname = ".pile.tmp";
    ioDelete(tempname.c_str());
    
    sprintf(buffer, "%s -o %s %s %s %s", path.c_str(), out.c_str(), objectstr.c_str(), options.c_str(), libraries.c_str());
    UI_print("Linking: %s\n", buffer);
    string buff = buffer;
    convertSlashes(buff);
    
    systemCall(buff);
    
    UI_print_file(tempname);
    ioDelete(tempname.c_str());
    
    UI_processEvents();
    UI_updateScreen();
    return NULL;
}

/*
Calls the compiler on the given source files, creating object files.

Takes: string (compiler path and name)
       list<string> (source file names)
       string (compiler flags)
Returns: true on success
         false on failure
*/
bool build(Environment& env, Configuration& config)
{
    char buffer[5000];
    string objName;
    string sourceFile;
    string tempname = ".pile.tmp";
    list<string> failedFiles;
    ioDelete(tempname.c_str());
    
    for(list<string>::iterator e = env.cflags.begin(); e != env.cflags.end(); e++)
    {
        config.cflags += " " + *e;
    }
    
    UI_debug_pile("Checking sources for building.\n");
    UI_debug_pile("Sources size: %d\n", env.sources.size());
    for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
    {
        if(!ioExists(*e))
        {
            UI_error("Source file \"%s\" not found.\n", e->c_str());
            continue;
        }
        sourceFile = quoteWhitespace(*e);
        FileData* fd = env.fileDataHash[*e];
        objName = getObjectName(*e, config.objPath, config.useSourceObjPath);
        mkpath(ioStripToDir(objName));
        objName = quoteWhitespace(objName);
        
        UI_debug_pile("Checking %s\n", e->c_str());
        
        // FIXME: mustRebuild() is crashing... Is it fixed yet?
        if(mustRebuild(objName, env.depends, fd))
        {
            sprintf(buffer, "%s %s -c %s -o %s", getCompiler(config, *e).c_str(), config.cflags.c_str(), sourceFile.c_str(), objName.c_str());
            
            
            string buff = buffer;
            convertSlashes(buff);
            
            UI_print(" Building %s\n  %s\n", e->c_str(), buff.c_str());
            
            
              // Append stdout and stderr to file
            //buff += ">> " + tempname + " 2>&1";
            
            //buff = "cmd.exe \"" + buff + "\"";
            
            UI_debug_pile("Actual call:\n %s\n", buff.c_str());
            //system(buff.c_str());
            //systemCall(("\"" + buff + "\"").c_str());
            
            // Needed for success check
            bool objExists = ioExists(objName);
            time_t objTime = 0;
            if(objExists)
                objTime = ioTimeModified(objName);
            
            systemCall(buff.c_str());
            
            UI_print_file(tempname);
            ioDelete(tempname.c_str());
            
            // Check to see if the build was successful.
            if(objExists)
            {
                // FIXME: This looks like it could fail if the build is quick (and the object had been modified...)!!!
                if(!ioExists(objName) || objTime >= ioTimeModified(objName))
                    failedFiles.push_back(*e);
            }
            else
            {
                if(!ioExists(objName))
                    failedFiles.push_back(*e);
            }
        }
        else
        {
            UI_print(" Up to date: %s\n", e->c_str());
        }
        if(UI_processEvents() < 0)
            return true;
        UI_updateScreen();
    }
    
    if(failedFiles.size() > 0)
    {
        UI_error("Some files failed to build:\n");
        for(list<string>::iterator e = failedFiles.begin(); e != failedFiles.end(); e++)
        {
            UI_error("  %s\n", e->c_str());
        }
        UI_error("\n");
        return false;
    }
    return true;
}

/*
Calls the linker on the given object files, creating the output (executable) file.

Takes: string (linker path and name)
       string (output file name)
       string (object file names)
       string (linker flags)
       string (linker libraries)
Returns: true on success
         false on failure
*/
bool link(const string& linker, Environment& env, Configuration& config)
{
    char buffer[5000];
    string out = quoteWhitespace(env.outfile + EXE_EXT);
    string tempname = ".pile.tmp";
    ioDelete(tempname.c_str());
    
    string objectstr;
    for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
    {
        objectstr += quoteWhitespace(getObjectName(*e, config.objPath, config.useSourceObjPath)) + " ";
    }
    for(list<string>::iterator e = env.objects.begin(); e != env.objects.end(); e++)
    {
        objectstr += quoteWhitespace(*e) + " ";
    }
    
    
    for(list<string>::iterator e = env.lflags.begin(); e != env.lflags.end(); e++)
    {
        config.lflags += " " + *e;
    }
    
    sprintf(buffer, "%s -o %s %s %s %s", linker.c_str(), out.c_str(), objectstr.c_str(), config.lflags.c_str(), config.libraries.c_str());
    UI_print("Linking: %s\n", buffer);
    string buff = buffer;
    convertSlashes(buff);
      // Append stdout and stderr to file
    //buff += ">> " + tempname + " 2>&1";
    //system(buff.c_str());
    systemCall(buff);
    
    UI_print_file(tempname);
    ioDelete(tempname.c_str());
    
    UI_processEvents();
    UI_updateScreen();
    return true;
}
