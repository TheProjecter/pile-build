#ifndef _PILE_LOAD_H__
#define _PILE_LOAD_H__

class Configuration;

bool loadPileFile(const string& filename, Configuration& config, string& outfile, list<string>& sources, list<string>& objects);

list<string> tokenize(string& line);

#endif
