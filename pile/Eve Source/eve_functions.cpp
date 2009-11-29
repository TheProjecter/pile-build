/*
eve_functions.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the implementation of the Function class.
*/

#include "eve_interpreter.h"
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"
#include <sstream>
#include <cassert>
#include <string>
using namespace std;

extern Interpreter interpreter;

// Functions for use in external functions

String* convertArg_String(Variable* arg)
{
    if(arg == NULL || arg->getType() != STRING)
    {
        interpreter.error("Error: Wrong type passed to function.  Expected string.\n");
        return NULL;
    }
    return static_cast<String*>(arg);
}

Array* convertArg_Array(Variable* arg, TypeEnum valueType)
{
    if(arg == NULL || arg->getType() != ARRAY)
    {
        interpreter.error("Error: Wrong type passed to function.  Expected array<%s>.\n", getTypeString(valueType).c_str());
        return NULL;
    }
    Array* a = static_cast<Array*>(arg);
    if(a->getValueType() != valueType)
    {
        interpreter.error("Error: Wrong type passed to function.  Expected array<%s>.\n", getTypeString(valueType).c_str());
        return NULL;
    }
    return a;
}

ClassObject* convertArg_ClassObject(Variable* arg, const std::string& className)
{
    if(arg == NULL || arg->getType() != CLASS_OBJECT)
    {
        interpreter.error("Error: Wrong type passed to function.  Expected object of class %s.\n", className.c_str());
        return NULL;
    }
    ClassObject* c = static_cast<ClassObject*>(arg);
    if(c->name != className)
    {
        interpreter.error("Error: Wrong type passed to function.  Expected object of class %s.\n", className.c_str());
        return NULL;
    }
    return c;
}



// ---------------------------------------

Function::Function(const std::string& text, const std::vector<TypeName>& argt, const std::string& value)
        : Variable(FUNCTION, text)
        , value(value)
        , argt(argt)
        , builtIn(FN_NONE)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, Variable* (*external_fn)())
        : Variable(FUNCTION, text)
        , builtIn(FN_EXTERNAL)
        , external_fn0(external_fn)
        , external_fn1(NULL)
        , external_fn2(NULL)
        , external_fn3(NULL)
        , external_fn4(NULL)
        , external_fn5(NULL)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, Variable* (*external_fn)(Variable*))
        : Variable(FUNCTION, text)
        , builtIn(FN_EXTERNAL)
        , external_fn0(NULL)
        , external_fn1(external_fn)
        , external_fn2(NULL)
        , external_fn3(NULL)
        , external_fn4(NULL)
        , external_fn5(NULL)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*))
        : Variable(FUNCTION, text)
        , builtIn(FN_EXTERNAL)
        , external_fn0(NULL)
        , external_fn1(NULL)
        , external_fn2(external_fn)
        , external_fn3(NULL)
        , external_fn4(NULL)
        , external_fn5(NULL)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*, Variable*))
        : Variable(FUNCTION, text)
        , builtIn(FN_EXTERNAL)
        , external_fn0(NULL)
        , external_fn1(NULL)
        , external_fn2(NULL)
        , external_fn3(external_fn)
        , external_fn4(NULL)
        , external_fn5(NULL)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*, Variable*, Variable*))
        : Variable(FUNCTION, text)
        , builtIn(FN_EXTERNAL)
        , external_fn0(NULL)
        , external_fn1(NULL)
        , external_fn2(NULL)
        , external_fn3(NULL)
        , external_fn4(external_fn)
        , external_fn5(NULL)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*, Variable*, Variable*, Variable*))
        : Variable(FUNCTION, text)
        , builtIn(FN_EXTERNAL)
        , external_fn0(NULL)
        , external_fn1(NULL)
        , external_fn2(NULL)
        , external_fn3(NULL)
        , external_fn4(NULL)
        , external_fn5(external_fn)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{}

Function::Function(const std::string& text, FunctionEnum builtIn)
        : Variable(FUNCTION, text)
        , builtIn(builtIn)
        , returnType("<temp>", NOT_A_TYPE)
        , lineNumber(0)
        , isMethod(false)
        , parentObject(NULL)
{
    switch(builtIn)
    {
        case FN_PRINT:
            argt.push_back(TypeName(STRING));
            break;
        case FN_PRINTLN:
            argt.push_back(TypeName(STRING));
            break;
        case FN_WARNING:
            argt.push_back(TypeName(STRING));
            break;
        case FN_ERROR:
            argt.push_back(TypeName(STRING));
            break;
        case FN_DEBUG:
            argt.push_back(TypeName(STRING));
            break;
        case FN_TYPE:
            argt.push_back(TypeName(VOID));
            break;
        case FN_STRING:
            argt.push_back(TypeName(VOID));
            break;
        case FN_BOOL:
            argt.push_back(TypeName(VOID));
            break;
        case FN_INT:
            argt.push_back(TypeName(VOID));
            break;
        case FN_FLOAT:
            argt.push_back(TypeName(VOID));
            break;
        case FN_INCLUDE:
            argt.push_back(TypeName(VOID));
            break;
        case FN_LS:
            argt.push_back(TypeName(VOID));
            break;
        case FN_DEFINED:
            argt.push_back(TypeName(STRING));
            break;
        case FN_COPY:
            argt.push_back(TypeName(STRING));
            argt.push_back(TypeName(STRING));
            break;
        case FN_MOVE:
            argt.push_back(TypeName(STRING));
            argt.push_back(TypeName(STRING));
            break;
        case FN_DELETE:
            argt.push_back(TypeName(STRING));
            break;
        case FN_MKDIR:
            argt.push_back(TypeName(STRING));
            break;
        case FN_MKPATH:
            argt.push_back(TypeName(STRING));
            break;
        case FN_MKFILE:
            argt.push_back(TypeName(STRING));
            break;
        case FN_CHMOD:
            argt.push_back(TypeName(STRING));
            argt.push_back(TypeName(STRING));
            break;
        case FN_MOD_TIME:
            argt.push_back(TypeName(STRING));
            break;
        case FN_EXTERNAL:
            break;
        case FN_NONE:
            break;
    }
}


Function::Function(const std::string& text, const Function& fn)
    : Variable(FUNCTION, text)
    , value(fn.value)
    , argt(fn.argt)
    , args(fn.args)
    , builtIn(fn.builtIn)
    , external_fn0(fn.external_fn0)
    , external_fn1(fn.external_fn1)
    , external_fn2(fn.external_fn2)
    , external_fn3(fn.external_fn3)
    , external_fn4(fn.external_fn4)
    , external_fn5(fn.external_fn5)
    , returnType(fn.returnType)
    , definitionFile(fn.definitionFile)
    , lineNumber(fn.lineNumber)
    , isMethod(fn.isMethod)
    , parentObject(fn.parentObject)
{
}

std::string& Function::getValue()
{
    return value;
}

std::vector<TypeName>& Function::getArgTypes()
{
    return argt;
}

void Function::setValue(const std::string& val)
{
    value = val;
}

void Function::setArgTypes(const std::vector<TypeName>& argTypes)
{
    argt = argTypes;
}

void Function::addArg(const TypeName& argType, const std::string& argName)
{
    argt.push_back(argType);
    args.push_back(argName);
}

/*void loadFromSig(const std::list<Token>& fnSignature)
{
    // ...
}*/
// Converts into a class method
void Function::makeAsMethod(std::string className)
{
    argt.insert(argt.begin(), CLASS_OBJECT);
    args.insert(args.begin(), className);  // It will be called 'this' by default
    isMethod = true;
}

std::string Function::getValueString()
{
    return value;
}

bool Function::isBuiltIn()
{
    return (builtIn != FN_NONE);
}

FunctionEnum Function::getBuiltIn()
{
    return builtIn;
}

// FIXME: Factor out similarities into a new function that both readFile() and Function::call() use.
// See readFile() since they're similar
Variable* Function::call(Interpreter& interpreter, std::vector<Variable*>& args)
{
    if(builtIn == FN_EXTERNAL)
    {
        if(external_fn0 != NULL && args.size() == 0)
            return external_fn0();
        if(external_fn1 != NULL && args.size() == 1)
            return external_fn1(args[0]);
        if(external_fn2 != NULL && args.size() == 2)
            return external_fn2(args[0], args[1]);
        if(external_fn3 != NULL && args.size() == 3)
            return external_fn3(args[0], args[1], args[2]);
        if(external_fn4 != NULL && args.size() == 4)
            return external_fn4(args[0], args[1], args[2], args[3]);
        if(external_fn5 != NULL && args.size() == 5)
            return external_fn5(args[0], args[1], args[2], args[3], args[4]);
        interpreter.error("Error calling external function.\n");
        return NULL;
    }
    if(builtIn != FN_NONE)
        return callBuiltIn(builtIn, args);
    // FIXME: Execute function body here
    
    //if(definitionFile == "")
    if(lineNumber == 0)
    {
        interpreter.error("Error: Function not defined.\n");
        return NULL;
    }
    
    
    interpreter.pushEnv(Scope(true));
    
    // Add the argument variables
    for(unsigned int i = 0; i < args.size(); i++)
    {
        if(isMethod && i == 0)
            interpreter.addVar("this", args[i]);
        else
            interpreter.addVar(this->args[i], args[i]);
    }
    
    
    
    //int lineNum = lineNumber;
    // FIXME: Hacky!
    // Make sure error messages use the right line numbers.
    unsigned int holdInterpreterLineNum = interpreter.lineNumber;
    unsigned int& lineNum = interpreter.lineNumber;
    lineNum = 1 + lineNumber;
    
    Token returnValue;
    
    // Interpret the function...
    std::stringstream str(getValue());
    bool continuation = false;
    bool wasTrueIf = false;
    bool wasFalseIf = false;
    list<Token> tokens;
    string line;
    while(!str.eof())
    {
        getline(str, line);
        
        if(!continuation)  // If we're not continuing the line, then clear the tokens.
            tokens.clear();
        
        list<Token> tok2 = tokenize1(line, continuation);
        
        tokens.splice(tokens.end(), tok2);
        
        if(!continuation || str.eof())  // Skip the eval if we're continuing, but not if the file ends!
        {
            returnValue = interpreter.evalTokens(tokens, true, wasTrueIf, wasFalseIf);
            wasTrueIf = wasFalseIf = false;
            if(returnValue.type == Token::KEYWORD)
            {
                if(returnValue.text == "true if")
                {
                    wasTrueIf = true;
                    wasFalseIf = false;
                }
                else if(returnValue.text == "false if")
                {
                    wasTrueIf = false;
                    wasFalseIf = true;
                }
                else if(returnValue.text == "return")
                {
                    break;
                }
            }
            if(returnValue.type == Token::VARIABLE && returnValue.var != NULL)
            {
                if(returnValue.var->getType() == CLASS)
                {
                    Class* aClass = dynamic_cast<Class*>(returnValue.var);
                    if(aClass != NULL)
                    {
                        interpreter.defineClass(aClass, &str);
                    }
                }
                else if(returnValue.var->getType() == FUNCTION)
                {
                    Function* aFn = dynamic_cast<Function*>(returnValue.var);
                    if(aFn != NULL)
                    {
                        interpreter.defineFunction(aFn, &str);
                    }
                }
            }
            lineNum++;
        }
        else if(continuation)
            lineNum++;  // Skip an extra line if the last one was continued...  This probably isn't right for multiple-line continues...
        
        returnValue.var = NULL;
    }
    
    interpreter.popEnv();
    
    if(returnValue.text != "return" && returnType.getValue() != VOID)
    {
        interpreter.error("Error: No return statement at end of non-void function.\n");
    }
    else
    {
        // Explicitly returning a value
        if(returnValue.var == NULL)
        {
            if(returnType.getValue() != VOID)
                interpreter.error("Error: Returning 'void' from a function which expects '%s'.\n", returnType.text.c_str());
        }
        else
        {
            if(returnValue.var->getTypeString() != returnType.text)
            {
                interpreter.error("Error: Returning type '%s' from a function which expects '%s'.\n", returnValue.var->getTypeString().c_str(), returnType.text.c_str());
            }
            UI_debug_pile("Returning type: %s\n", returnValue.var->getTypeString().c_str());
        }
    }
    
    interpreter.lineNumber = holdInterpreterLineNum;
    
    return returnValue.var;
}

Variable* Function::copy()
{
    Variable* cp = new Function(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}
