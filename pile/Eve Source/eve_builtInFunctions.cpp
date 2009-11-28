/*
eve_builtInFunctions.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the implementations for the functions that are built into
the Eve interpreter.
*/

#include "eve_interpreter.h"
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"
#include <cassert>
#include <string>
#include "../External Code/goodio.h"
using namespace std;

extern Interpreter interpreter;

/*
Returns a new string that has the printing escape sequences (\n, \t, etc.)
replaced with the appropriate characters.
*/
string replaceEscapes(string s)
{
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
    return s;
}

/*
Built-in print().
*/
void fn_print(Variable* arg)
{
    assert(arg->getType() == STRING);
    
    UI_print("%s", replaceEscapes(static_cast<String*>(arg)->getValue()).c_str());
}


/*
Built-in println().
*/
void fn_println(Variable* arg)
{
    if(arg->getType() == STRING)
    {
        string s = static_cast<String*>(arg)->getValue();
        // Every string in here has double quotes (so it's at least size 2)
        // Maybe I should move this out to the tokenizing or eval step.
        //s = s.substr(1, s.size()-2);
        
        s = replaceEscapes(s);
        
        // Do it
        UI_print("%s\n", s.c_str());
    }
    else
    {
        interpreter.error("Error: println() was not given a string.\n");
    }
}

/*
Built-in warning().
*/
void fn_warning(Variable* arg)
{
    if(arg->getType() == STRING)
    {
        string s = static_cast<String*>(arg)->getValue();
        // Every string in here has double quotes (so it's at least size 2)
        // Maybe I should move this out to the tokenizing or eval step.
        //s = s.substr(1, s.size()-2);
        
        s = replaceEscapes(s);
        
        // Do it
        UI_warning("%s", s.c_str());
    }
    else
    {
        interpreter.error("Error: warning() was not given a string.\n");
    }
}

/*
Built-in error().
*/
void fn_error(Variable* arg)
{
    if(arg->getType() == STRING)
    {
        string s = static_cast<String*>(arg)->getValue();
        // Every string in here has double quotes (so it's at least size 2)
        // Maybe I should move this out to the tokenizing or eval step.
        //s = s.substr(1, s.size()-2);
        
        s = replaceEscapes(s);
        
        // Do it
        UI_error("%s", s.c_str());
    }
    else
    {
        interpreter.error("Error: error() was not given a string.\n");
    }
}


/*
Built-in debug().
*/
void fn_debug(Variable* arg)
{
    if(arg->getType() == STRING)
    {
        string s = static_cast<String*>(arg)->getValue();
        // Every string in here has double quotes (so it's at least size 2)
        // Maybe I should move this out to the tokenizing or eval step.
        //s = s.substr(1, s.size()-2);
        
        s = replaceEscapes(s);
        
        // Do it
        UI_debug("%s", s.c_str());
    }
    else
    {
        interpreter.error("Error: debug() was not given a string.\n");
    }
}


/*
Built-in cast to bool.
*/
Bool* boolCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Bool("<temp>", b->getValue());
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Bool("<temp>", i->getValue());
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Bool("<temp>", f->getValue());
    }
    return NULL;
}

/*
Built-in cast to int.
*/
Int* intCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Int("<temp>", int(b->getValue()));
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Int("<temp>", i->getValue());
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Int("<temp>", int(f->getValue()));
    }
    return NULL;
}


/*
Built-in cast to float.
*/
Float* floatCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Float("<temp>", float(b->getValue()));
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Float("<temp>", float(i->getValue()));
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Float("<temp>", f->getValue());
    }
    return NULL;
}


/*
Built-in external file include().
*/
Void* include(Variable* v)
{
    if(v->getType() != STRING)
    {
        return NULL;
    }
    String* s = static_cast<String*>(v);
    string file = s->getValue();
    
    // Save old stuff
    string oldFile = interpreter.currentFile;
    unsigned int oldLine = interpreter.lineNumber;
    bool oldErrorFlag = interpreter.errorFlag;
    
    // Call the interpreter on this file.
    interpreter.readFile(file);
    
    // Restore old stuff
    interpreter.currentFile = oldFile;
    interpreter.lineNumber = oldLine;
    interpreter.errorFlag = oldErrorFlag;
    
    return NULL;
}

/*
Returns true if 'str' matches 'exp', which can contain wildcards '*'.
*/
bool matchWildcard(const string& exp, const string& str)
{
    unsigned int i = 0;
    
    list<string> parts = ioExplode(exp, '*');
    
    bool first = (*(parts.begin()) != "");
    for(list<string>::iterator e = parts.begin(); e != parts.end(); e++)
    {
        unsigned int j = str.find(*e);
        
        // This makes sure that the beginning is not a wildcard
        if(first && j != 0)
            return false;
        else
            first = false;
        
        // This makes sure that the end is not a wildcard
        list<string>::iterator f = e;
        f++;
        if(f == parts.end())
        {
            if(*e != "" && j != str.size() - e->size())
                return false;
        }
        
        if(j == string::npos || i > j)
            return false;
        i = j;
    }
    
    
    return true;
}

/*
Built-in ls().
*/
Array* fn_ls(Variable* v)
{
    if(v->getType() != STRING)
    {
        return NULL;
    }
    String* s = static_cast<String*>(v);
    string text = s->getValue();
    
    // FIXME: Assumes a single directory (no wildcard in directory)
    string dir = ioStripToDir(text);
    if(dir == "")
        dir = ".";
    
    string files = ioStripToFile(text);
    
    
    Array* result = new Array("<temp>", STRING);
    
    list<string> l = ioList(dir, false, true);
    
    if(dir != "")
    {
        if(dir == ".")
            dir = "";
        else
            dir += "/";
    }
    
    for(list<string>::iterator e = l.begin(); e != l.end(); e++)
    {
        if(matchWildcard(files, *e))
            result->push_back(new String("<temp>", dir + *e));
    }
    
    return result;
}

/*
Built-in defined().
*/
Bool* fn_defined(Variable* v)
{
    Bool* result = new Bool(false);
    
    result->setValue((v != NULL && interpreter.getVar(v->text)));
    
    return result;
}

/*
Built-in copy().
*/
Variable* fn_copy(Variable* f, Variable* d)
{
    String* file = dynamic_cast<String*>(f);
    String* dest = dynamic_cast<String*>(d);
    if(file != NULL && dest != NULL)
        ioCopy(file->getValue(), dest->getValue());
    
    return NULL;
}

/*
Built-in move().
*/
Variable* fn_move(Variable* f, Variable* d)
{
    String* file = dynamic_cast<String*>(f);
    String* dest = dynamic_cast<String*>(d);
    if(file != NULL && dest != NULL)
        ioMove(file->getValue(), dest->getValue());
    
    return NULL;
}

/*
Built-in delete().
*/
Variable* fn_delete(Variable* f)
{
    String* file = dynamic_cast<String*>(f);
    if(file != NULL)
        ioDelete(file->getValue());
    
    return NULL;
}

/*
Built-in mkdir().
*/
Variable* fn_mkdir(Variable* f)
{
    String* file = dynamic_cast<String*>(f);
    if(file != NULL)
        ioNewDir(file->getValue());
    
    return NULL;
}

/*
Built-in mkpath().
*/
Variable* fn_mkpath(Variable* f)
{
    String* file = dynamic_cast<String*>(f);
    if(file == NULL)
        return NULL;
        
    list<string> l = ioExplode(file->getValue(), '/');
    string dir;
    for(list<string>::iterator e = l.begin(); e != l.end(); e++)
    {
        dir += *e + "/";
        ioNewDir(dir);
    }
    
    return NULL;
}

/*
Built-in mkfile().
*/
Variable* fn_mkfile(Variable* f)
{
    String* file = dynamic_cast<String*>(f);
    
    if(file != NULL)
        ioNew(file->getValue());
    
    return NULL;
}

/*
Built-in chmod().
*/
Variable* fn_chmod(Variable* f, Variable* p)
{
    String* file = dynamic_cast<String*>(f);
    String* perms = dynamic_cast<String*>(p);
    
    // FIXME: Implement!
    if(file != NULL && perms != NULL)
        ;
    
    return NULL;
}

/*
Built-in mod_time().
*/
Variable* fn_mod_time(Variable* f)
{
    String* file = dynamic_cast<String*>(f);
    
    if(file != NULL)
        return new Int("<temp>", ioTimeModified(file->getValue()));
    return new Int("<temp>", -1);
}


/*
Groups together the calling of built-in functions.  Used in callFn().
*/
Variable* callBuiltIn(FunctionEnum fn, std::vector<Variable*>& args)
{
    Variable* result = NULL;
    
    switch(fn)
    {
        case FN_PRINT:
            if(args.size() != 1)
                return NULL;
            fn_print(args[0]);
            break;
        case FN_PRINTLN:
            if(args.size() != 1)
                return NULL;
            fn_println(args[0]);
            break;
        case FN_WARNING:
            if(args.size() != 1)
                return NULL;
            fn_warning(args[0]);
            break;
        case FN_ERROR:
            if(args.size() != 1)
                return NULL;
            fn_error(args[0]);
            break;
        case FN_DEBUG:
            if(args.size() != 1)
                return NULL;
            fn_debug(args[0]);
            break;
        case FN_TYPE:
            if(args.size() != 1)
                return NULL;
            result = new TypeName("<temp>", args[0]);
            break;
        case FN_STRING:
            if(args.size() != 1)
                return NULL;
            result = new String("<temp>", args[0]->getValueString());
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
        case FN_INCLUDE:
            if(args.size() != 1)
                return NULL;
            result = include(args[0]);
            break;
        case FN_LS:
            if(args.size() != 1)
                return NULL;
            result = fn_ls(args[0]);
            break;
        case FN_DEFINED:
            if(args.size() != 1)
                return NULL;
            result = fn_defined(args[0]);
            break;
        case FN_COPY:
            if(args.size() != 2)
                return NULL;
            result = fn_copy(args[0], args[1]);
            break;
        case FN_MOVE:
            if(args.size() != 2)
                return NULL;
            result = fn_move(args[0], args[1]);
            break;
        case FN_DELETE:
            if(args.size() != 1)
                return NULL;
            result = fn_delete(args[0]);
            break;
        case FN_MKDIR:
            if(args.size() != 1)
                return NULL;
            result = fn_mkdir(args[0]);
            break;
        case FN_MKPATH:
            if(args.size() != 1)
                return NULL;
            result = fn_mkpath(args[0]);
            break;
        case FN_MKFILE:
            if(args.size() != 1)
                return NULL;
            result = fn_mkfile(args[0]);
            break;
        case FN_CHMOD:
            if(args.size() != 2)
                return NULL;
            result = fn_chmod(args[0], args[1]);
            break;
        case FN_MOD_TIME:
            if(args.size() != 1)
                return NULL;
            result = fn_mod_time(args[0]);
            break;
        case FN_EXTERNAL:
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
