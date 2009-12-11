/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

string_functions.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains random, useful functions for manipulating strings.
*/

#include "External Code/goodio.h"
#include "pile_system.h"
#include "string_functions.h"
using namespace std;

bool isWhitespace(const char& c);

/*


Takes: string (text)
Returns: true if the string has the right first character
         false otherwise
*/
bool firstChar(const string& text, const char& c)
{
    return (text.size() > 0 && text[0] == c);
}


void toLower(string& text)
{
    for(unsigned int i = 0; i < text.size(); i++)
        text[i] = tolower(text[i]);
}


string getBaseName(string file)
{
    unsigned int dotpos = file.find_last_of(".");
    if(dotpos != string::npos)
        file = file.substr(0, dotpos);
    return file;
}

string addDirSlash(string path)
{
    if(path.size() > 0 && path[path.size()-1] != '/')
        return path + '/';
    return path;
}

string getObjectName(string source, string objPath, bool useSourceDir)
{
    string objName = ioStripToFile(source);
    objName = getBaseName(objName) + ".o";

    string dir = ioStripToDir(source);

    if(useSourceDir)
    {
        return (addDirSlash(dir) + addDirSlash(objPath) + objName);
    }
    else
    {
        return (addDirSlash(objPath) + addDirSlash(dir) + objName);
    }
}

// Remove leading and ending whitespace and add double quotes if there is interstitial whitespace
string quoteWhitespace(string str)
{
    while(str.size() > 0 && isWhitespace(*(str.begin())))
        str.erase(str.begin());
    while(str.size() > 0 && isWhitespace(*(str.end()-1)))
        str.erase(str.end()-1);

    if(str.size() > 0 && str[0] != '\"' && str[str.size()-1] != '\"'
       && str.find_first_of(' ') != string::npos)
        str = '\"' + str + '\"';

    return str;
}

// Quotes an empty string too.
string quoteThis(string str)
{
    while(str.size() > 0 && isWhitespace(*(str.begin())))
        str.erase(str.begin());
    while(str.size() > 0 && isWhitespace(*(str.end()-1)))
        str.erase(str.end()-1);
    if(str.size() == 0)
        return "\"\"";
    if(str[0] != '\"' && str[str.size()-1] != '\"')
        str = '\"' + str + '\"';

    return str;
}


string removeQuotes(string str)
{
    while(str.size() > 0 && *(str.begin()) == '\"')
        str.erase(str.begin());
    while(str.size() > 0 && *(str.end()-1) == '\"')
        str.erase(str.end()-1);

    return str;
}



/*
Creates a name for an executable by changing the file extension.

Takes: string (file names to be converted)
Returns: string (converted name)
*/
string getExeName(string file)
{
    unsigned int dotpos = file.find_last_of(".");
    if(dotpos != string::npos)
    {
        return (file.substr(0, dotpos) + EXE_EXT);
    }
    else
    {
        return (file + EXE_EXT);
    }
}



void removePath(string& file)
{
    unsigned int lastSlash = file.find_last_of('/');
    if(lastSlash != string::npos)
    {
        file = file.substr(lastSlash+1, string::npos);
    }
}

/*
Combines two lists to make a full list of object file names.

Takes: list<string> (source file names to convert into object file names)
       list<string> (object file names)
Returns: string (All object file names separated by spaces)
*/
string getObjectString(const list<string>& sources, const list<string>& objects)
{
    string objectstr;

    for(list<string>::const_iterator e = sources.begin(); e != sources.end(); e++)
    {
        string obj = *e;
        unsigned int dotpos = e->find_last_of(".");
        if(dotpos != string::npos)  // Perhaps unneccessary
        {
            obj = obj.substr(0, dotpos) + ".o";
            removePath(obj);
            objectstr += (obj + " ");
        }
    }

    for(list<string>::const_iterator e = objects.begin(); e != objects.end(); e++)
    {
        objectstr += (*e + " ");
    }

    return objectstr;
}


bool isCExt(const string& ext)
{
    return (ext == ".c" || ext == ".c86");
}

bool isCPPExt(const string& ext)
{
    return (ext == ".cpp" || ext == ".cxx" || ext == ".cc" || ext == ".c++");
}

bool isFORTRANExt(const string& ext)
{
    return (ext == ".f" || ext == ".f77" || ext == ".f90" || ext == ".for");
}

/*
Tells whether the given file name is a source file or not.

Takes: string (file name)
Returns: true if file is a source file
         false if file is not a source file
*/
bool isSourceFile(const string& file)
{
    unsigned int dotpos = file.find_last_of(".");
    if(dotpos != string::npos)
    {
        string ext = file.substr(dotpos, string::npos);
        toLower(ext);
        if(ext == ".c" || ext == ".c86" || ext == ".cpp" || ext == ".cxx" || ext == ".cc")
            return true;
    }
    return false;
}



