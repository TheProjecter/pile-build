#ifndef _PILE_COMMANDS_H__
#define _PILE_COMMANDS_H__


bool mkpath(const string& path);
bool clean(bool cleanall, const list<string>& sources, Configuration& config, const string& outfile);
bool cleanOld(bool cleanall, const list<string>& sources, Configuration& config, const string& outfile);
bool edit(string file, Configuration& config);
string getVersion();



#endif
