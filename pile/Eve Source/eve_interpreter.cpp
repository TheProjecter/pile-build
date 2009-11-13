// FIXME: Remove Pile dependence
#include "eve_interpreter.h"
#include <fstream>
using namespace std;

extern Interpreter interpreter;



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
            arr = new Array(type);
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






// This kinda belongs in eve_variables.cpp, but it is so similar to readFile()...
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

