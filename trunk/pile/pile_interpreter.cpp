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
