/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_commands.h

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

Header for pile_commands.cpp
*/

#ifndef _PILE_COMMANDS_H__
#define _PILE_COMMANDS_H__


bool mkpath(const string& path);
bool clean(bool cleanall, const list<string>& sources, Configuration& config, const string& outfile);
bool cleanOld(bool cleanall, const list<string>& sources, Configuration& config, const string& outfile);
bool edit(string file, Configuration& config);
string getVersion();



#endif
