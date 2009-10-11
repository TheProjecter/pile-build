#include "External Code/goodio.h"
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
    
    if(str.find_first_of(' ') != string::npos)
        str = '\"' + str + '\"';
    
    return str;
}

