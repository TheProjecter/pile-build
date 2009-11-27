/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_interpreter.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the interpreter interface.
*/

#include "pile_global.h"
#include "pile_env.h"
#include "Eve Source/eve_interpreter.h"


Interpreter interpreter;



bool interpret(string filename, Environment& env, Configuration& config)
{
    // Put Pile variables into the interpreter.
    env.initInterpreter(interpreter, config);
    
    if(!interpreter.readFile(filename))
        return false;
    interpreter.printEnv();
    
    interpreter.popAll();
    
    // Get the Pile variables back
    env.finalizeInterpreter(interpreter);
    return true;
}
