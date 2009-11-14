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







