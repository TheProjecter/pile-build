/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_build.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the functions which compile and link source files.  They are
added to the global scope of the interpreter.
*/

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



void changeVariableFromCommandLine(Variable* var, const std::string& name, const std::string& value)
{
    if(var == NULL)
        return;

    list<string> l = ioExplode(value, ',');
    if(l.size() > 1)
    {
        if(var->getType() == ARRAY)
        {
            Array* arr = static_cast<Array*>(var);
            for(list<string>::iterator e = l.begin(); e != l.end(); e++)
            {
                if(arr->getValueType() == STRING)
                    arr->push_back(new String("<temp>", *e));
                else
                {
                    UI_warning("Warning: Command-line value does not have the appropriate type for variable '%s'.\n", name.c_str());
                    return;
                }
            }
        }
        else if(var->getType() == LIST)
        {
            //List* arr = static_cast<List*>(var);
            for(list<string>::iterator e = l.begin(); e != l.end(); e++)
            {
                // FIXME: Implement List loading!
                /*if(arr->getValueType() == STRING)
                    arr->push_back(new String("<temp>", *e));
                else*/
                {
                    UI_warning("Warning: Command-line value does not have the appropriate type for variable '%s'.\n", name.c_str());
                    return;
                }
            }
        }
        else
        {
            UI_warning("Warning: Command-line value does not have the appropriate type for variable '%s'.\n", name.c_str());
            return;
        }
    }
    else if(l.size() == 1)
    {
        string newValue = *(l.begin());
        // FIXME: ONLY ACCEPTS STRINGS!
        if(var->getType() == STRING)
        {
            String* str = static_cast<String*>(var);
            str->setValue(newValue);
        }
        else if(var->getType() == ARRAY)
        {
            Array* arr = static_cast<Array*>(var);

            if(arr->getValueType() == STRING)
                arr->push_back(new String("<temp>", newValue));
            else
            {
                UI_warning("Warning: Command-line value does not have the appropriate type for variable '%s'.\n", name.c_str());
                return;
            }
        }
        else if(var->getType() == LIST)
        {
            //List* arr = static_cast<List*>(var);

            /*if(arr->getValueType() == STRING)
                arr->push_back(new String("<temp>", newValue));
            else*/
            {
                UI_warning("Warning: Command-line value does not have the appropriate type for variable '%s'.\n", name.c_str());
                return;
            }
        }
        else
        {
            UI_warning("Warning: Command-line value does not have the appropriate type for variable '%s'.\n", name.c_str());
            return;
        }
    }
    else
    {
        UI_warning("Warning: Invalid command-line value for variable '%s'.\n", name.c_str());
        return;
    }
}


Variable* createNewVariableFromCommandLine(const std::string& name, const std::string& value)
{
    // Command line value can look like: 46 or value or value1,value2,value3

    list<string> l = ioExplode(value, ',');
    if(l.size() > 1)
    {
        Array* arr = new Array(name, STRING);
        for(list<string>::iterator e = l.begin(); e != l.end(); e++)
        {
            arr->push_back(new String("<temp>", *e));
        }
        return arr;
    }

    // FIXME: ONLY ACCEPTS STRINGS!
    String* str = new String(name, value);
    return str;
}


void checkSourceExistence(list<string>& sources)
{
    for(list<string>::iterator e = sources.begin(); e != sources.end();)
    {
        if(!ioExists(*e))
        {
            UI_warning("%s not found!  pile will ignore it.\n", e->c_str());
            sources.erase(e);
            e = sources.begin();
        }
        else
            e++;
    }
}


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
    ClassObject* c = convertArg_ClassObject(arg1, "Compiler");
    Array* sources = convertArg_Array(arg2, STRING);

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

        recurseIncludes(env.depends, env.fileDataHash, config.includePaths, removeQuotes(sourceFile), "");
    }

    return NULL;
}

// Returns array<string> objectFiles
// Takes ClassObject compiler, array<string> sourceFiles, array<string> options
Variable* fn_build(Variable* arg1, Variable* arg2, Variable* arg3)
{
    ClassObject* c = convertArg_ClassObject(arg1, "Compiler");
    Array* sources = convertArg_Array(arg2, STRING);
    Array* opts = convertArg_Array(arg3, STRING);

    if(c == NULL || sources == NULL || opts == NULL)
        return NULL;

    string path = quoteWhitespace(static_cast<String*>(c->getVariable("path"))->getValue());
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
        if(mustRebuild(removeQuotes(objName), env.depends, fd))
        {
            sprintf(buffer, "%s %s -c %s -o %s", path.c_str(), options.c_str(), sourceFileQuoted.c_str(), objName.c_str());


            string buff = buffer;
            convertSlashes(buff);

            UI_print(" Building %s\n  %s\n", sourceFile.c_str(), buff.c_str());


            UI_debug_pile("Actual call:\n %s\n", buff.c_str());

            // Needed for success check
            /*bool objExists = ioExists(objName);
            time_t objTime = 0;
            if(objExists)
                objTime = ioTimeModified(objName);*/

            int result = systemCall(buff.c_str());

            UI_print_file(tempname);
            ioDelete(tempname.c_str());
			
			if(result != 0)
				failedFiles.push_back(sourceFile);

            // Check to see if the build was successful.
            /*if(objExists)
            {
                // FIXME: This looks like it could fail if the build is quick (and the object had been modified...)!!!
                if(!ioExists(objName) || objTime >= ioTimeModified(objName))
                    failedFiles.push_back(sourceFile);

            }
            else
            {
                if(!ioExists(objName))
                    failedFiles.push_back(sourceFile);
            }*/
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
        // FIXME: I have to make a better way to signal an error.
        resultObjects->setValue(vector<Variable*>());
    }

    return resultObjects;
}


// Returns VOID (NULL)
// Params: ClassObject linker, string outfile, array objects, array libraries, array options
Variable* fn_link(Variable* arg1, Variable* arg2, Variable* arg3, Variable* arg4, Variable* arg5)
{
    ClassObject* c = convertArg_ClassObject(arg1, "Linker");
    String* outname = convertArg_String(arg2);
    Array* objs = convertArg_Array(arg3, STRING);
    Array* libs = convertArg_Array(arg4, STRING);
    Array* opts = convertArg_Array(arg5, STRING);

    if(c == NULL || outname == NULL || objs == NULL || opts == NULL)
        return NULL;
    
    if(objs->size() == 0)
        return NULL;

    string path = quoteWhitespace(static_cast<String*>(c->getVariable("path"))->getValue());
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

    int result = systemCall(buff);

    UI_print_file(tempname);
    ioDelete(tempname.c_str());
	
	if(result != 0)
        UI_error("Linking failed.\n");

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

    map<string, string>::iterator fl = env.variables.find("CFLAGS");
    if(fl != env.variables.end())
    {
        list<string> l = ioExplode(fl->second, ',');
        for(list<string>::iterator e = l.begin(); e != l.end(); e++)
        {
            if(*e != "")
                config.cflags += " " + *e;
        }
    }

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

    map<string, string>::iterator fl = env.variables.find("LFLAGS");
    if(fl != env.variables.end())
    {
        list<string> l = ioExplode(fl->second, ',');
        for(list<string>::iterator e = l.begin(); e != l.end(); e++)
        {
            if(*e != "")
                config.lflags += " " + *e;
        }
    }
    //UI_print("LFLAGS: %s\n", config.lflags.c_str());
    //for(map<string, string>::iterator e = env.variables.begin(); e != env.variables.end(); e++)
    //    UI_print("%s: %s\n", e->first.c_str(), e->second.c_str());


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
