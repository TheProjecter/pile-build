/*
eve_interpreter.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the implementation for the some of the Interpreter methods,
including readFile(), which coordinates parsing of Eve files.
*/

// FIXME: Remove Pile dependence
#include "eve_interpreter.h"
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"
#include <fstream>
#include "stdarg.h"
using namespace std;

extern Interpreter interpreter;



Scope::Scope(bool isolated)
        : isolated(isolated)
        , state(NO_BLOCK)
        , singleLine(false)
{}
Scope::Scope(bool isolated, ScopeEnum state, bool singleLine)
        : isolated(isolated)
        , state(state)
        , singleLine(singleLine)
{}

Variable* Scope::getVar(const std::string& name)
{
    if (env.find(name) == env.end())
        return NULL;
    return env[name];
}

void Scope::printEnv()
{
    UI_debug_pile("Printing scope... %d variables.\n", env.size());
    for (std::map<std::string, Variable*>::iterator e = env.begin(); e != env.end(); e++)
    {
        UI_debug_pile(" '%s' has type '%s' with value '%s'\n", e->first.c_str(), e->second->getTypeString().c_str(), e->second->getValueString().c_str());
    }
}


Outputter::Outputter(std::string syntax)
{
    setSyntax(syntax);
}

void Outputter::setSyntax(std::string syn)
{
    syntax = syn;
}

void Outputter::error(std::string file, unsigned int line, std::string message)
{
    // FIXME: Syntax must be in this order: file, line, message
    char buff[syntax.size() + file.size() + message.size() + 100];
    sprintf(buff, "%s", syntax.c_str());
    UI_error(buff, file.c_str(), line, message.c_str());
}



Interpreter::Interpreter()
    : lineNumber(1)
    , errorFlag(false)
    //, outputter("%s: @%d:%s")
    , outputter("%s:%d:%s")
{
    pushEnv(true);  // Push global scope
    
    // Add built-in functions
    Scope& s = *(env.begin());
    s.env["print"] = new Function("print", FN_PRINT);
    s.env["println"] = new Function("println", FN_PRINTLN);
    s.env["warning"] = new Function("warning", FN_PRINT);
    s.env["error"] = new Function("error", FN_PRINT);
    s.env["debug"] = new Function("debug", FN_PRINT);
    s.env["type"] = new Function("type", FN_TYPE);
    s.env["bool"] = new Function("bool", FN_BOOL);
    s.env["int"] = new Function("int", FN_INT);
    s.env["float"] = new Function("float", FN_FLOAT);
    s.env["string"] = new Function("string", FN_STRING);
    s.env["include"] = new Function("include", FN_INCLUDE);
    s.env["ls"] = new Function("ls", FN_LS);
    s.env["defined"] = new Function("defined", FN_DEFINED);
    
    array_size = new Function("size", &Array::size_fn);
    array_size->isMethod = true;
}

void Interpreter::addClass(Class* c)
{
    if(c == NULL)
    {
        error("Error: Error adding class definition.\n");
        return;
    }
    
    for(std::list<Class*>::iterator e = classDefs.begin(); e != classDefs.end(); e++)
    {
        if((*e)->name == c->name)
        {
            error("Error: Class already defined.\n");
            return;
        }
    }
    
    classDefs.push_back(c);
}

void Interpreter::printEnv()
{
    UI_debug_pile("Printing Interpreter Environment... %d scopes.\n", env.size());
    for (std::list<Scope>::iterator e = env.begin(); e != env.end(); e++)
    {
        e->printEnv();
    }
}

Variable* Interpreter::callFn(Function* fn, std::list<Token>& arguments)
{
    UI_debug_pile("  Calling a function.\n");
    // Sort out the arguments...
    
    // Put all the tokens between commas into new lists
    // Evaluate those lists
    // Make argument variables and assign the values
    // Parse the function.
    
    if(fn == NULL)
        return NULL;
    
    std::vector<Variable*> args;
    std::list<Token> tokens;
    int argNum = 1;
    
    std::list<Token>::iterator f = arguments.begin();  // Mark the beginning
    
    int num = 0;  // Keeps track of parentheses
    for(std::list<Token>::iterator e = arguments.begin(); e != arguments.end();)
    {
        if(e->type == Token::SEPARATOR)
        {
            if(e->sep == OPEN_PARENTHESIS)
                num++;
            else if(e->sep == CLOSE_PARENTHESIS)
                num--;
            // If we're not in parens and it's a comma, take and evaluate the tokens.
            else if(num == 0 && e->sep == COMMA)
            {
                tokens.clear();
                tokens.splice(tokens.begin(), arguments, f, e);
                
                Token eval = evalTokens(tokens, false, false, false, true);
                
                if(eval.var != NULL)
                {
                    args.push_back(eval.var);
                }
                else
                {
                    error("Error: Argument %d of function call is invalid.\n", argNum);
                    return NULL;
                }
                argNum++;
                
                f = e;  // Mark the beginning
                f++;  // Skip the comma
            }
        }
        
        e++;
        if(e == arguments.end())
        {
            tokens.clear();
            
            tokens.splice(tokens.begin(), arguments, f, e);
            
            Token eval = evalTokens(tokens, false, false, false, true);
            
            if(eval.var != NULL)
            {
                args.push_back(eval.var);
            }
            else
            {
                error("Error: Argument %d of function call is invalid.\n", argNum);
                return NULL;
            }
            argNum++;
            
            f = e;  // Mark the beginning
            f++;  // Skip the comma
        }
    }
    
    if(fn->isMethod && fn->parentObject != NULL)
    {
        args.insert(args.begin(), fn->parentObject);
    }
    
    // Compare number of arguments
    if(fn->getBuiltIn() == FN_EXTERNAL)
        return fn->call(*this, args);
    if(args.size() > fn->getArgTypes().size())
    {
        error("Error: Too many arguments in function call.\n");
        return NULL;
    }
    if(args.size() < fn->getArgTypes().size())
    {
        error("Error: Too few arguments in function call.\n");
        return NULL;
    }
    
    for(unsigned int i = 0; i < args.size(); i++)
    {
        // Void will accept anything
        if(fn->getArgTypes()[i].getValue() != VOID && !isConvertable(args[i]->getType(), fn->getArgTypes()[i].getValue()))
        {
            error("Error: Argument %d has the wrong type in function call.\n", i+1);
            return NULL;
        }
        // If it must be a reference, make sure that it is.
        if(fn->getArgTypes()[i].reference)
        {
            if(!args[i]->reference)
            {
                error("Error: Argument %d must be a reference in function call.\n", i+1);
                return NULL;
            }
        }
        else  // If it is not a reference, pass it by value.
        {
            args[i] = args[i]->copy();
            // This is a new variable declaration, so it will now be a reference within the function
            args[i]->reference = true;
        }
    }
    
    return fn->call(*this, args);
}

Variable* Interpreter::evaluateExpression(Variable* A, OperatorEnum operation)
{
    if (operation == NEGATE)
        return ::negate(A);
    //else if (operation == NOT)
    //    return ::not_(A);
    error("Error: Undefined operation\n");
    return A;
}

Variable* Interpreter::evaluateExpression(Variable* A, OperatorEnum operation, Variable* B)
{
    if (operation == EQUALS)
        return comparison(A, B, EQUALS);
    if (operation == NOT_EQUALS)
        return comparison(A, B, NOT_EQUALS);
    if (operation == LESS_EQUAL)
        return comparison(A, B, LESS_EQUAL);
    if (operation == GREATER_EQUAL)
        return comparison(A, B, GREATER_EQUAL);
    if (operation == LESS)
        return comparison(A, B, LESS);
    if (operation == GREATER)
        return comparison(A, B, GREATER);
    if (operation == NOT_LESS)
        return comparison(A, B, NOT_LESS);
    if (operation == NOT_GREATER)
        return comparison(A, B, NOT_GREATER);
    if (operation == AND)
        return comparison(A, B, AND);
    if (operation == OR)
        return comparison(A, B, OR);
    if (operation == ASSIGN)
        return assign(A, B);
    if (operation == ADD)
        return add(A, B);
    if (operation == ADD_ASSIGN)
        return add_assign(A, B);
    if (operation == SUBTRACT_ASSIGN)
        return subtract_assign(A, B);
    if (operation == MULTIPLY_ASSIGN)
        return multiply_assign(A, B);
    if (operation == DIVIDE_ASSIGN)
        return divide_assign(A, B);
    if (operation == MODULUS_ASSIGN)
        return modulus_assign(A, B);
    if (operation == EXPONENTIATE_ASSIGN)
        return exponentiate_assign(A, B);
    if (operation == SUBTRACT)
        return subtract(A, B);
    if (operation == MULTIPLY)
        return multiply(A, B);
    if (operation == DIVIDE)
        return divide(A, B);
    if (operation == MODULUS)
        return ::modulus(A, B);
    if (operation == EXPONENTIATE)
        return exponentiate(A, B);
    if (operation == DOT)
        return dot(A, B);
    if (operation == ARRAY_ACCESS)
        return array_access(A, B);
    if (operation == HAS_ELEMENT)
        return has_element(A, B);
    if (operation == NOT_HAS_ELEMENT)
        return not_has_element(A, B);
    error("Error: Undefined operation\n");
    return A;
}


// Environment interface

std::map<std::string, Variable*>* Interpreter::topEnv()
{
    return &((env.begin())->env);
}

bool Interpreter::addVar(const std::string& name, Variable* var)
{
    UI_debug_pile("In addVar()...\n");
    std::map<std::string, Variable*>* e = topEnv();
    if (e->find(name) == e->end())
    {
        UI_debug_pile("... Setting variable\n");
        (*e)[name] = var;
        return true;
    }
    error("Error: Variable \"%s\" redefined!\n", name.c_str());
    return false;
}

bool Interpreter::setVar(const std::string& name, Variable* var)
{
    std::map<std::string, Variable*>* e = topEnv();
    if (e->find(name) == e->end())
    {
        error("Error: Variable \"%s\" not defined!\n", name.c_str());
        return false;
    }
    *((*e)[name]) = *var;
    return true;
}

Variable* Interpreter::getVar(const std::string& name)
{
    Variable* result = NULL;
    for (std::list<Scope>::iterator e = env.begin(); e != env.end(); e++)
    {
        result = e->getVar(name);
        if (result != NULL)
            return result;
        // result is NULL now
        if (e->isolated)
        {
            e++;
            if (e != env.end())
            {
                // Check global scope
                e = env.end();
                e--;
                result = e->getVar(name);
            }
            break;
        }
    }

    if (result == NULL)
        ;//error("Error: Variable '%s' not found at this scope\n", name.c_str());

    return result;
}

void Interpreter::pushEnv(const Scope& scope)
{
    env.push_front(scope);
}

void Interpreter::pushEnv(bool isolated)
{
    env.push_front(Scope(isolated));
}

Scope& Interpreter::currentScope()
{
    return (env.front());
}

void Interpreter::popEnv()
{
    UI_debug_pile("Popping environment: %d scopes\n", env.size());
    if (env.size() > 1)
        env.erase(env.begin());
    UI_debug_pile("Popped environment: %d scopes\n", env.size());
}

void Interpreter::popAll()
{
    while (env.size() > 1)
        env.erase(env.begin());
}









void Interpreter::error(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;
    char buffer[2000];
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    outputter.error(currentFile, lineNumber, buffer);
    //UI_error("@%d: %s", interpreter.lineNumber, buffer);
    
    errorFlag = true;
}


bool Interpreter::readFile(string filename)
{
    currentFile = filename;
    lineNumber = 1;
    errorFlag = false;
    
    ifstream fin;
    fin.open(filename.c_str());
    if(fin.fail())
    {
        fin.close();
        currentFile = "";
        return false;
    }
    
    bool continuation = false;
    bool wasTrueIf = false;
    bool wasFalseIf = false;
    list<Token> tokens;
    string line;
    while(!fin.eof())
    {
        getline(fin, line);
        
        if(!continuation)  // If we're not continuing the line, then clear the tokens.
            tokens.clear();
        
        list<Token> tok2 = tokenize1(line, continuation);
        
        tokens.splice(tokens.end(), tok2);
        
        if(!continuation || fin.eof())  // Skip the eval if we're continuing, but not if the file ends!
        {
            Token result = evalTokens(tokens, true, wasTrueIf, wasFalseIf);
            
            wasTrueIf = wasFalseIf = false;
            if(result.type == Token::KEYWORD)
            {
                if(result.text == "true if")
                {
                    wasTrueIf = true;
                    wasFalseIf = false;
                }
                else if(result.text == "false if")
                {
                    wasTrueIf = false;
                    wasFalseIf = true;
                }
            }
            if(result.type == Token::VARIABLE && result.var != NULL)
            {
                UI_debug_pile("Returned a variable.\n");
                if(result.var->getType() == CLASS)
                {
                    Class* aClass = dynamic_cast<Class*>(result.var);
                    if(aClass != NULL)
                    {
                        defineClass(aClass, &fin);
                        addClass(aClass);
                    }
                }
                else if(result.var->getType() == FUNCTION)
                {
                    // FIXME: This happens whenever a function is the result??
                    // I might want to add Token::SPECIAL so I can define classes/functions
                    Function* aFn = dynamic_cast<Function*>(result.var);
                    if(aFn != NULL)
                    {
                        UI_debug_pile("Defining function.\n");
                        defineFunction(aFn, &fin);
                    }
                }
            }
            lineNumber++;
        }
        else if(continuation)
            lineNumber++;  // Skip an extra line if the last one was continued...  This probably isn't right for multiple-line continues...
    }
    
    fin.close();
    currentFile = "";
    
    return !errorFlag;
}











Variable* Interpreter::getArrayLiteral(list<Token>& tokens, list<Token>::iterator& e)
{
    Array* arr = NULL;
    
    TypeEnum type = NOT_A_TYPE;
    
    for(; e != tokens.end(); e++)
    {
        if(e->type == Token::SEPARATOR)
        {
            if(e->sep == OPEN_SQUARE_BRACKET)  // FIXME: This should allow nesting later...
                continue;
            if(e->sep == CLOSE_SQUARE_BRACKET)  // FIXME: This should allow nesting later...
                break;
        }
        
        // Get the thing between [ and , or ]
        list<Token> tok2;
        
        while(e != tokens.end())
        {
            if(e->type == Token::SEPARATOR && (e->sep == COMMA || e->sep == CLOSE_SQUARE_BRACKET))
                break;
            tok2.push_back(*e);
            e++;
        }
        
        Token t = evalTokens(tok2, false, false, false, true);
        if(t.type == Token::OPERATOR)
        {
            interpreter.error("Error: Unexpected operator when reading an array literal.\n");
            continue;
        }
        if(t.type == Token::SEPARATOR)
        {
            interpreter.error("Error: Unexpected separator when reading an array literal.\n");
            continue;
        }
        if(t.var == NULL)
        {
            interpreter.error("Error: NULL var in token...\n");
            continue;
        }
        if(type == NOT_A_TYPE)
        {
            type = t.var->getType();
            if(type == NOT_A_TYPE || type == VOID)
            {
                type = NOT_A_TYPE;
                interpreter.error("Error: Invalid type in array literal.\n");
                continue;
            }
            arr = new Array("<temp>", type);
        }
        if(t.var->getType() != type)
        {
            interpreter.error("Error: Inconsistent type in array literal.\n");
            continue;
        }
        
        arr->push_back(t.var);
        
        if(e->type == Token::SEPARATOR && e->sep == CLOSE_SQUARE_BRACKET)
            break;
    }
    
    // Skip the closing bracket
    //if(e != tokens.end())
    //    e++;
    
    if(arr != NULL)
    {
        UI_debug_pile("  Returning an array: Size = %d\n", arr->getValue().size());
        for(unsigned int i = 0; i < arr->getValue().size(); i++)
        {
            UI_debug_pile("    Element: %s\n", arr->getValue()[i]->getValueString().c_str());
        }
    }
    
    return arr;
}







