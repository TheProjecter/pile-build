#ifndef _PILE_BUILD_H__
#define _PILE_BUILD_H__


#include <list>
#include <string>
#include "pile_env.h"
#include "pile_config.h"

void checkSourceExistence(std::list<std::string>& sources);
bool build(Environment& env, Configuration& config);
bool link(const std::string& linker, Environment& env, Configuration& config);



#endif
