#ifndef _STRING_FUNCTIONS_H__
#define _STRING_FUNCTIONS_H__

#include <string>

bool firstChar(const std::string& text, const char& c);

void toLower(std::string& text);

std::string getObjectName(std::string source, std::string objPath, bool useSourceDir = false);

std::string addDirSlash(std::string path);

std::string getBaseName(std::string file);

std::string quoteWhitespace(std::string str);

#endif
