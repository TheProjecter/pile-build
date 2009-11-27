/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_config.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the functions which parse the configuration file.
*/

#include "pile_global.h"
#include "External Code/goodio.h"
#include "pile_config.h"
#include "pile_ui.h"
#include "string_functions.h"
#include <fstream>

string getVersion();

string strip(string str, char c);
bool isWhitespace(const char& c);
bool isMatch(const char& c, const vector<char>& matching);
bool isQuantizer(const char& c);
string nextToken(string& line);
list<string> tokenize(string& line);

char removeQuantifiers(string& str);


/*
Creates a new config file with default values which depend on the operating system.

Takes: string (Config file location)
       Configuration (config settings)
Returns: true on success
         false on failure
*/
bool createConfig(string path, Configuration& config)
{
    UI_log("Creating config\n");
    ofstream fout((path + "pile.conf").c_str(), ios::trunc);
    if(fout.fail())
    {
    UI_log("Failed.\n");
        fout.close();
        return false;
    }
    
    UI_log("Writing config\n");
    fout << "pile Version " << getVersion() << endl;
    UI_log("Wrote version\n");
    
    UI_log("Writing editor\n");
    if(config.editor == "vi")
    {
        fout << "# Notice: The default editor is 'vi'." << endl
             << "#  If you don't have experience with vi, it is recommended that" << endl
             << "#  you change the default editor." << endl
             << "#  vi controls:" << endl
             << "#    Press 'esc' to make sure that you're in Command mode" << endl
             << "#    Quitting: Type :q to quit or type :x to save and quit." << endl
             << "#    Deleting: Press 'x' or 'X' to delete characters." << endl
             << "#    Deleting: Press 'dd' to delete a line." << endl
             << "#    Editing: Press 'i' to enter Insert mode." << endl;
    }
    UI_log("Wrote vi warning\n");
    
    fout << "editor: " << config.editor << endl;
    UI_log("Wrote editor\n");
    
    fout << "installPath: " << quoteWhitespace(config.installPath) << endl;
    
    fout << "lang C: " << config.languages.find("C_COMPILER")->second << ", "
                  << config.languages.find("C_LINKER_D")->second << ", "
                  << config.languages.find("C_LINKER_S")->second << ", "
                  << config.languages.find("C_SYNTAX")->second << endl;
    fout << "lang C++: " << config.languages.find("CPP_COMPILER")->second << ", "
                  << config.languages.find("CPP_LINKER_D")->second << ", "
                  << config.languages.find("CPP_LINKER_S")->second << ", "
                  << config.languages.find("CPP_SYNTAX")->second << endl;
    fout << "lang FORTRAN: " << config.languages.find("FORTRAN_COMPILER")->second << ", "
                  << config.languages.find("FORTRAN_LINKER_D")->second << ", "
                  << config.languages.find("FORTRAN_LINKER_S")->second << ", "
                  << config.languages.find("FORTRAN_SYNTAX")->second << endl;
    
    fout << "includeDirs:";
    for(list<string>::iterator e = config.includePaths.begin(); e != config.includePaths.end(); e++)
    {
        fout << " " << *e;
    }
    fout << endl;
    
    UI_log("Done writing config\n");
    fout.close();
    return true;
}



/*
Changes the config settings according to the settings found in a line of text.

Takes: 
Returns: true on success
         false on failure
*/
bool parseConfigLine(string& line, Configuration& config)
{
    list<string> tokens = tokenize(line);
    
    for(list<string>::iterator e = tokens.begin(); e != tokens.end();)
    {
        if(*e == "Editor:" || *e == "editor:")
        {
            e++;
            if(e != tokens.end())
            {
                config.editor = *e;
            }
            else
                break;
        }
        else if(*e == "includeDirs:" || *e == "incDir:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                    config.includePaths.push_back(*e);
                e++;
            }
            continue;
        }
        else if(*e == "libraryDirs:" || *e == "libDir:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                    config.libPaths.push_back(*e);
                e++;
            }
            continue;
        }
        else if(*e == "objectDir:" || *e == "objDir:")
        {
            e++;
            if(e != tokens.end())
            {
                config.objPath =*e;
            }
            else
                break;
        }
        else if(*e == "useSourceObjectDir" || *e == "useSourceObjectPath")
        {
            e++;
            if(e == tokens.end() || *e != "=")
                break;
            e++;
            if(e != tokens.end())
            {
                if(*e == "true" || *e == "TRUE" || *e == "1")
                {
                    config.useSourceObjPath = true;
                }
                else if(*e == "false" || *e == "FALSE" || *e == "0")
                    config.useSourceObjPath = false;
            }
            else
                break;
        }
        else if(*e == "useAutoDepend")
        {
            e++;
            if(e == tokens.end() || *e != "=")
                break;
            e++;
            if(e != tokens.end())
            {
                if(*e == "true" || *e == "TRUE" || *e == "1")
                {
                    config.useAutoDepend = true;
                }
                else if(*e == "false" || *e == "FALSE" || *e == "0")
                    config.useAutoDepend = true;
            }
            else
                break;
        }
        else if(*e == "installPath:" || *e == "installDir:")
        {
            e++;
            if(e != tokens.end())
            {
                config.installPath = *e;
            }
            else
                break;
        }
        
        e++;
    }
    
    return true;
}

/*
Loads the config settings from pile.conf.

Takes: 
Returns: true on success
         false on failure
*/
bool parseConfig(const string& filename, Configuration& config)
{
    ifstream fin;
    fin.open(filename.c_str());
    
    if(fin.fail())
    {
        fin.close();
        return false;
    }
    
    
    string c;
    int i = 0;
    while(!fin.eof())
    {
        getline(fin, c);
        if(!parseConfigLine(c, config))
        {
            UI_error("pile Error: Bad syntax on line %d of %s\n", i, filename.c_str());
        }
    }
    
    fin.close();
    return true;
}

/*
Fills in the lists of language, compiler, and linkers from the config file.
Also creates a config file if none exists.

Takes: string (config file location)
       Configuration (config settings)
Returns: true on success
         false on failure
*/
bool loadConfig(string path, Configuration& config)
{
    if(!ioExists((path + "pile.conf")))
    {
        UI_warning("pile config file not found.\n");
        if(!createConfig(path, config))
            UI_error("pile error: Failed to create config file.\n");
        else
            UI_warning("Created config file successfully.\n");
    }
    else
    {
        // Load it
        if(!parseConfig(path + "pile.conf", config))
        {
            UI_error("pile Warning: Failed to load config file.\n");
        }
    }
    return false;
}
