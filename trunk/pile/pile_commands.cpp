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
        system((config.editor + " " + file).c_str());
        return true;
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
