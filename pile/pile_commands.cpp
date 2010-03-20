/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_commands.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the functions which implement several command-line options.
*/

#include "pile_global.h"
#include "pile_config.h"
#include "pile_ui.h"
#include "External Code/goodio.h"
#include "Eve Source/eve_interpreter.h"


extern Interpreter interpreter;

void stripWrappingWhitespace(string& s);


string getObjectName(string source, string objPath, bool useSourceDir);


string getVersion()
{
    char buff[64];
    sprintf(buff, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUGFIX);
    return buff;
}

/*
Creates the necessary directories to make the given path valid.

Takes: string (a directory path)
Returns: true on success
         false on failure
*/

bool mkpath(const string& path)
{
    // Find the first /
    // Get the substring 0, i
    // Make the directory
    // Find the next /
    // Get the substring old, i - old
    // make it
    unsigned int i = 0;
    string dir;
    do
    {
        i = path.find_first_of('/', i+1);
        dir = path.substr(0, i);
        if(dir != "")
            ioNewDir(dir);
    }
    while(i != string::npos);
    
    return ioExists(path);
}

/*
Deletes temporary files.

Takes: bool (if true, the output file will be deleted)
       list<string> (file names for removing object files)
       string (the output file)
Returns: true on success
         false on failure
*/
bool clean(bool cleanall, const list<string>& sources, Configuration& config, const string& outfile)
{
    //list<string> ls = ioList(".", false, true);
    
    bool result = true;
    
    if(cleanall && !ioDelete(outfile))
        result = false;
    
    string objName;
    for(list<string>::const_iterator e = sources.begin(); e != sources.end(); e++)
    {
        objName = getObjectName(*e, config.objPath, config.useSourceObjPath);
        if(!ioDelete(objName))
            result = false;
    }
    
    return result;
}


bool compare_time_lowtohigh(string first, string second)
{
    return ioTimeModified(first) < ioTimeModified(second);
}

/*
Deletes object files that are old according to the largest time spacing between
any two of the files.

Takes: bool (if true, the output file will be deleted)
       list<string> (file names for removing object files)
       string (the output file)
Returns: true on success
         false on failure
*/
bool cleanOld(bool cleanall, const list<string>& sources, Configuration& config, const string& outfile)
{
    list<string> objects;
    
    bool result = true;
    
    if(cleanall && !ioDelete(outfile))
        result = false;
    
    string obj;
    for(list<string>::const_iterator e = sources.begin(); e != sources.end(); e++)
    {
        obj = getObjectName(*e, config.objPath, config.useSourceObjPath);
        if(ioExists(obj))
            objects.push_back(obj);
    }
    
    objects.sort(compare_time_lowtohigh);
    
    time_t prev_time = 0;
    time_t max_delta = 0;
    time_t time_gap = 0;
    for(list<string>::iterator e = objects.begin(); e != objects.end(); e++)
    {
        time_t this_time = ioTimeModified(*e);
        
        if(prev_time != 0)
        {
            if(this_time - prev_time > max_delta)
            {
                max_delta = this_time - prev_time;
                time_gap = this_time;
            }
        }
        prev_time = this_time;
    }
    
    for(list<string>::iterator e = objects.begin(); e != objects.end(); e++)
    {
        if(ioTimeModified(*e) < time_gap)
        {
            if(!ioDelete(*e))
                result = false;
        }
    }
    
    return result;
}

/*
Opens the default editor on a file.

Takes: string (file name)
       map<string, string> (config settings)
Returns: true on success
         false on failure
*/
bool edit(string file, Configuration& config)
{
    UI_print("pile: Editing %s\n", file.c_str());
    
    if(config.editor == "")
    {
        UI_error("pile error: No default editor has been set.\n");
        return false;
    }
    else
    {
        return (system((config.editor + " " + file).c_str()) == 0);
    }
}

/*
Searches the source files for dependencies and puts them into the pilefile.

Takes: -
Returns: true on success
         false on failure
*/
bool scan()
{
    return false;
}

/*
Installs executables, libraries, and headers to the system.

Takes: -
Returns: true on success
         false on failure
*/
bool install()
{
    return false;
}








// Returns void
// Takes array<string> sourceFiles, string type
Variable* fn_codeStats(Variable* arg1, Variable* arg2)
{
    Array* sourceFiles = convertArg_Array(arg1, STRING);
    String* typeStr = convertArg_String(arg2);

    if(sourceFiles == NULL || typeStr == NULL)
        return NULL;

    vector<Variable*> sources = sourceFiles->getValue();
    string type = typeStr->getValue();
    
    /* Count the:
    Number of files
    Number of lines
    Number and percentage of blank lines
    Number and percentage of comment lines
    Number and percentage of code lines
    Number and percentage of lines that have mixed code and comments
    Total text lines
    */
    
    if(type == "C++" || type == "c++" || type == "C" || type == "c")
    {
        unsigned int numFiles = 0;
        unsigned int numLines = 0;
        unsigned int numBlanks = 0;
        unsigned int numComments = 0;
        unsigned int numCodes = 0;
        unsigned int numPreprocessor = 0;
        unsigned int numMixed = 0;
        unsigned int numText = 0;
        unsigned int numStatements = 0;
        
        bool inQuote = false;
        bool inPreprocessor = false;  // For multi-line macros
        bool inComment = false;
        
        for(vector<Variable*>::iterator e = sources.begin(); e != sources.end(); e++)
        {
            if((*e)->getType() != STRING)
            {
                interpreter.error("Wrong type in array sent to codeStats().\n");
                return NULL;
            }
            
            String* fileStr = convertArg_String(*e);
            if(fileStr == NULL)
            {
                interpreter.error("Wrong type in array sent to codeStats().\n");
                return NULL;
            }
            string file = fileStr->getValue();
            ioFileReader r(file);
            numFiles++;
            
            while(r.ready())
            {
                string line = r.getLine();
                stripWrappingWhitespace(line);
                numLines++;
                if(line.size() > 0)
                    numText++;
                
                /*for(string::iterator f = line.begin(); f != line.end(); f++)
                {
                    if(inQuote)
                    {
                        if(*f == '\"')
                            inQuote = false;
                    }
                    else if(inPreprocessor)
                    {
                        string::iterator g = line.end();
                        g--;
                        if(f == g && *f != '\\')
                            inPreprocessor = false;
                    }
                }*/
            }
            
        }
        
        UI_print("In %d files...\nTotal lines: %d\nTotal code lines: %d\n", numFiles, numLines, numText);
        
        
    }
    
    
    return NULL;
}

