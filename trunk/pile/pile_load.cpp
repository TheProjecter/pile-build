/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_load.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the deprecated pilefile parsing system.
*/

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

