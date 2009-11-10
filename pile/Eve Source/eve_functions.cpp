#include "eve_interpreter.h"

#include <string>
using namespace std;

extern Interpreter interpreter;



void fn_print(Variable* arg)
{
    if(arg->getType() == STRING)
    {
        string s = static_cast<String*>(arg)->getValue();
        // Every string in here has double quotes (so it's at least size 2)
        // Maybe I should move this out to the tokenizing or eval step.
        //s = s.substr(1, s.size()-2);
        
        // Replace the escape sequences...
        for(string::iterator e = s.begin(); e != s.end();)
        {
            if(*e == '\\')
            {
                string::iterator f = e;
                //UI_debug_pile("String: %s\n", s.c_str());
                //e++;  // Get the next character
                s.erase(f);  // Erase the backslash
                //UI_debug_pile("Erased1 String: %s\n", s.c_str());
                if(e != s.end())
                {
                    //UI_debug_pile("Checking escape sequence.\n");
                    if(*e == 'n')
                    {
                        //UI_debug_pile("Replacing escape sequence.\n");
                        f = e;
                        //e++;
                        s.erase(f);
                        //UI_debug_pile(" Erased2 String: %s\n", s.c_str());
                        s.insert(e, '\n');
                        //UI_debug_pile(" Inserted String: %s\n", s.c_str());
                        e++;
                    }
                    else  // Unknown escape sequence...
                    {
                        UI_warning(" Warning: Unknown escape sequence, '\\%c'", *e);
                        f = e;
                        //e++;
                        s.erase(f);  // Erase the escaped character
                    }
                }
            }
            else
                e++;
        }
        
        // Do it
        UI_print("%s", s.c_str());
    }
    else
    {
        interpreter.error("Error: print() was not given a string.\n");
    }
}

void fn_println(Variable* arg)
{
    if(arg->getType() == STRING)
    {
        string s = static_cast<String*>(arg)->getValue();
        // Every string in here has double quotes (so it's at least size 2)
        // Maybe I should move this out to the tokenizing or eval step.
        //s = s.substr(1, s.size()-2);
        
        // Replace the escape sequences...
        for(string::iterator e = s.begin(); e != s.end();)
        {
            if(*e == '\\')
            {
                string::iterator f = e;
                //UI_debug_pile("String: %s\n", s.c_str());
                //e++;  // Get the next character
                s.erase(f);  // Erase the backslash
                //UI_debug_pile("Erased1 String: %s\n", s.c_str());
                if(e != s.end())
                {
                    //UI_debug_pile("Checking escape sequence.\n");
                    if(*e == 'n')
                    {
                        //UI_debug_pile("Replacing escape sequence.\n");
                        f = e;
                        //e++;
                        s.erase(f);
                        //UI_debug_pile(" Erased2 String: %s\n", s.c_str());
                        s.insert(e, '\n');
                        //UI_debug_pile(" Inserted String: %s\n", s.c_str());
                        e++;
                    }
                    else  // Unknown escape sequence...
                    {
                        UI_warning(" Warning: Unknown escape sequence, '\\%c'", *e);
                        f = e;
                        //e++;
                        s.erase(f);  // Erase the escaped character
                    }
                }
            }
            else
                e++;
        }
        
        // Do it
        UI_print("%s\n", s.c_str());
    }
    else
    {
        interpreter.error("Error: println() was not given a string.\n");
    }
}


Bool* boolCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Bool(b->getValue());
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Bool(i->getValue());
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Bool(f->getValue());
    }
    return NULL;
}

Int* intCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Int(int(b->getValue()));
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Int(i->getValue());
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Int(int(f->getValue()));
    }
    return NULL;
}

Float* floatCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Float(float(b->getValue()));
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Float(float(i->getValue()));
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Float(f->getValue());
    }
    return NULL;
}


Variable* callBuiltIn(FunctionEnum fn, vector<Variable*>& args)
{
    Variable* result = NULL;
    
    switch(fn)
    {
        case FN_PRINT:
            if(args.size() != 1)
                return NULL;
            fn_print(args[0]);
            result = new Void;
            break;
        case FN_PRINTLN:
            if(args.size() != 1)
                return NULL;
            fn_println(args[0]);
            result = new Void;
            break;
        case FN_TYPE:
            if(args.size() != 1)
                return NULL;
            result = new String(args[0]->getTypeString());
            break;
        case FN_STRING:
            if(args.size() != 1)
                return NULL;
            result = new String(args[0]->getValueString());
            break;
        case FN_BOOL:
            if(args.size() != 1)
                return NULL;
            result = boolCast(args[0]);
            break;
        case FN_INT:
            if(args.size() != 1)
                return NULL;
            result = intCast(args[0]);
            break;
        case FN_FLOAT:
            if(args.size() != 1)
                return NULL;
            result = floatCast(args[0]);
            break;
        default:
            break;
    }
    if(result != NULL)
    {
        UI_debug_pile("Built-in fn result: '%s' (%s)\n", result->getTypeString().c_str(), result->getValueString().c_str());
    }
    return result;
}
