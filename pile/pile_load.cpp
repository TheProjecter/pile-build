#include "pile_global.h"
#include "pile_config.h"
#include "pile_ui.h"
#include <fstream>

/*
Removes the given character from the string, wherever it appears.

Takes: string (the source string)
       char (the character to erase)
Returns: string (the new string)
*/
string strip(string str, char c)
{
    for(string::iterator e = str.begin(); e != str.end();)
    {
        if(*e == c)
        {
            str.erase(e);
            e = str.begin();
        }
        e++;
    }
    return str;
}

/*
Tells whether or not a given character is whitespace.

Takes: char (the character to test)
Returns: true if the character is whitespace
         false if the character is not whitespace
*/
bool isWhitespace(const char& c)
{
    return (c == ' ' || c == '\n' || c == '\t');
}

bool isMatch(const char& c, const vector<char>& matching)
{
    unsigned int num = matching.size();
    if(num == 0)
        return false;
    switch(c)
    {
        case '{':
            return (matching[num-1] == '}');
        case '[':
            return (matching[num-1] == ']');
        case '`':
            return (matching[num-1] == '`');
        case '(':
            return (matching[num-1] == ')');
        case '\'':
            return (matching[num-1] == '\'');
        case '\"':
            return (matching[num-1] == '\"');
        case '<':
            return (matching[num-1] == '>');
    }
    return false;
}

bool isQuantizer(const char& c)
{
    switch(c)
    {
        case '{':
            return true;
        case '[':
            return true;
        case '`':
            return true;
        case '(':
            return true;
        case '\'':
            return true;
        case '\"':
            return true;
        case '<':
            return true;
    }
    return false;
}

/*
Returns the next whitespace-delimited token and erases that token from the original string.
(destructive)

Takes: string (a raw line of text)
Returns: string (the next whitespace-delimited token)
*/
string nextToken(string& line)
{
    // Ignore whitespace until you find something else.  Then find the whitespace on the other side
    // and return that string.
    int found = -1;
    vector<char> matching;
    for(unsigned int i = 0; i < line.size(); i++)
    {
        if(matching.size() > 0)
        {
            if(isMatch(line[i], matching))
            {
                char matcher = line[i];
                matching.erase(matching.end()-1);
                if(matching.size() == 0)
                {
                    string token = line.substr(found, i+1 - found);
                    line.erase(0, i+1);
                    if(matcher == '\"')
                    {
                        token.erase(token.begin());
                        token.erase(token.end()-1);
                    }
                    return token;
                }
            }
            continue;
        }
        if(line[i] == '#')
        {
            if(found < 0)
            {
                line.clear();
                return "";
            }
            string token = line.substr(found, i - found);
            line.clear();
            return token;
        }
        
        if(found < 0)
        {
            if(isWhitespace(line[i]))
                continue;
            else
            {
                found = i;
                if(isQuantizer(line[i]))
                {
                    matching.push_back(line[i]);
                }
            }
        }
        else
        {
            if(isWhitespace(line[i]))
            {
                string token = line.substr(found, i - found);
                line.erase(0, i+1);
                return token;
            }
            if(i+1 >= line.size())
            {
                string token = line.substr(found, i+1 - found);
                line.erase(0, i+1);
                return token;
            }
        }
    }

    return "";
}

/*
Breaks a raw line of text into whitespace-delimited tokens.

Takes: string (line of text)
Returns: list<string> (separated tokens)
*/
list<string> tokenize(string& line)
{
    list<string> tokens;
    string str;
    #ifdef PILE_DEBUG_TOKENS
        static int tokenNum = 1;
    #endif
    
    do
    {
        str = nextToken(line);
        #ifdef PILE_DEBUG_TOKENS
            UI_debug_pile("Token %d: \"%s\"\n", tokenNum, str.c_str());
            tokenNum++;
        #endif
        if(str != "")
            tokens.push_back(str);
    }
    while(line.size() > 0 && str != "");
    
    return tokens;
}

/*
Changes the config settings according to the settings found in a line of text.

Takes: 
Returns: true on success
         false on failure
*/
bool parseLine(string& line, Configuration& config, string& outfile, list<string>& sources, list<string>& objects)
{
    list<string> tokens = tokenize(line);
    
    for(list<string>::iterator e = tokens.begin(); e != tokens.end();)
    {
        if(*e == "out:" || *e == "output:")
        {
            e++;
            if(e != tokens.end())
            {
                outfile = *e;
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
                config.objPath = *e;
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
        else if(*e == "sources:" || *e == "src:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                    sources.push_back(*e);
                e++;
            }
            continue;
        }
        else if(*e == "libraries:" || *e == "libs:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                    config.libraries += " " + *e;
                e++;
            }
            continue;
        }
        else if(*e == "cflags:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                    config.cflags += " " + *e;
                e++;
            }
            continue;
        }
        else if(*e == "lflags:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                    config.lflags += " " + *e;
                e++;
            }
            continue;
        }
        else if(*e == "flags:")
        {
            e++;
            while(e != tokens.end())
            {
                if(*e != "")
                {
                    config.cflags += " " + *e;
                    config.lflags += " " + *e;
                }
                e++;
            }
            continue;
        }
        
        e++;
    }
    
    return true;
}

/*
Loads the config settings from a pilefile.

Takes: 
Returns: true on success
         false on failure
*/
bool loadPileFile(const string& filename, Configuration& config, string& outfile, list<string>& sources, list<string>& objects)
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
        if(!parseLine(c, config, outfile, sources, objects))
        {
            UI_error("pile Error: Bad syntax on line %d of %s\n", i, filename.c_str());
        }
    }
    
    fin.close();
    return true;
}
