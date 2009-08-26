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
    list<string> ls = ioList(".", false, true);
    
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

bool clean(string ext)
{
    list<string> ls = ioList(".", false, true);
    
    bool result = true;
    
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        unsigned int dotpos = e->find_last_of(".");
        if(dotpos != string::npos && e->substr(dotpos, string::npos) == ext)
        {
            UI_print(("Cleaning " + *e + "...").c_str());
            if(!ioIsWriteable(*e) || !ioDelete(*e))
            {
                result = false;
                UI_print(" Error");
                UI_log(("Cleaning " + *e + "... Error\n").c_str());
            }
            UI_print("\n");
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
