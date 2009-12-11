/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

string_functions.h

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

Header for string_functions.cpp
*/

#ifndef _STRING_FUNCTIONS_H__
#define _STRING_FUNCTIONS_H__

#include <list>
#include <string>

bool firstChar(const std::string& text, const char& c);

void toLower(std::string& text);

std::string getObjectName(std::string source, std::string objPath, bool useSourceDir = false);

std::string addDirSlash(std::string path);

std::string getBaseName(std::string file);

std::string quoteWhitespace(std::string str);

std::string quoteThis(std::string str);

std::string removeQuotes(std::string str);

/*
Creates a name for an executable by changing the file extension.

Takes: string (file names to be converted)
Returns: string (converted name)
*/
std::string getExeName(std::string file);

void removePath(std::string& file);

/*
Combines two lists to make a full list of object file names.

Takes: list<string> (source file names to convert into object file names)
       list<string> (object file names)
Returns: string (All object file names separated by spaces)
*/
std::string getObjectString(const std::list<std::string>& sources, const std::list<std::string>& objects);


bool isCExt(const std::string& ext);

bool isCPPExt(const std::string& ext);

bool isFORTRANExt(const std::string& ext);

/*
Tells whether the given file name is a source file or not.

Takes: string (file name)
Returns: true if file is a source file
         false if file is not a source file
*/
bool isSourceFile(const std::string& file);

#endif
