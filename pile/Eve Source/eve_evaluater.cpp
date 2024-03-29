/*
eve_evaluater.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the interpreter's workhorse token evaluater and the functions
that are called to evaluate the definitions of user-defined functions and classes.
*/

#include "eve_interpreter.h"
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"
#include <cassert>
#include <sstream>

#include <list>
using namespace std;

Bool* boolCast(Variable* v);

TypeEnum getTypeFromString(const string& str);

/*
Reads a function definition from the input stream.
*/
bool Interpreter::defineFunction(Variable* functionvar, std::istream* stream)
{
    // FIXME: Assumes there are no prototypes for now...
    
    Function* fn = dynamic_cast<Function*>(functionvar);
    assert(fn != NULL);
    
    fn->setValue("");
    fn->definitionFile = currentFile;
    
    int curlies = 0;
    
    bool done = false;
    bool continuation = false;
    list<Token> tokens;
    string line;
    while(!stream->eof() && !done)
    {
        getline(*stream, line);
        string lineCopy = line;
        
        if(!continuation)  // If we're not continuing the line, then clear the tokens.
            tokens.clear();
        
        list<Token> tok2 = tokenize1(line, continuation);
        
        tokens.splice(tokens.end(), tok2);
        
        // This is used to remove the leading '{' line and trailing '}' line.
        // That avoids pushing an extra scope.
        bool addThis = true;
        
        if(!continuation || stream->eof())  // Skip the eval if we're continuing, but not if the file ends!
        {
            for(list<Token>::iterator e = tokens.begin(); e != tokens.end();)
            {
                if(e->type == Token::SEPARATOR)
                {
                    if(e->sep == OPEN_CURLY_BRACKET)
                    {
                        if(curlies == 0)
                            addThis = false;
                        curlies++;
                    }
                    else if(e->sep == CLOSE_CURLY_BRACKET)
                    {
                        curlies--;
                        if(curlies == 0)
                            addThis = false;
                    }
                }
                
                if(curlies <= 0)
                {
                    done = true;
                    break;
                }
                
                e++;
            }
            
            lineNumber++;
        }
        else if(continuation)
            lineNumber++;  // Skip an extra line if the last one was continued...  This probably isn't right for multiple-line continues...
            
        // Assumes curly brackets are on an otherwise empty line
        if(addThis)
            fn->setValue(fn->getValue() + "\n" + lineCopy);
    }
    
    
    return true;
}


/*
Reads a class definition from the input stream.
*/
bool Interpreter::defineClass(Variable* classvar, istream* stream)
{
    Class* theClass = dynamic_cast<Class*>(classvar);
    assert(theClass != NULL);
    
    int curlies = 0;
    
    bool done = false;
    bool continuation = false;
    list<Token> tokens;
    string line;
    while(!stream->eof() && !done)
    {
        getline(*stream, line);
        
        if(!continuation)  // If we're not continuing the line, then clear the tokens.
            tokens.clear();
        
        list<Token> tok2 = tokenize1(line, continuation);
        
        tokens.splice(tokens.end(), tok2);
        
        if(!continuation || stream->eof())  // Skip the eval if we're continuing, but not if the file ends!
        {
            for(list<Token>::iterator e = tokens.begin(); e != tokens.end();)
            {
                if(e->type == Token::SEPARATOR)
                {
                    if(e->sep == OPEN_CURLY_BRACKET)
                        curlies++;
                    else if(e->sep == CLOSE_CURLY_BRACKET)
                        curlies--;
                }
                
                if(curlies <= 0)
                {
                    done = true;
                    break;
                }
                
                if(e->type == Token::VARIABLE)
                {
                    if (e->var->getType() == TYPENAME)
                    {
                        TypeName* t = static_cast<TypeName*>(e->var);
                        TypeEnum newType = t->getValue();
                        
                        e++;
                        if(e == tokens.end() || e->type != Token::VARIABLE)
                        {
                            error("Error: Loose type name.\n");
                            break;
                        }
                        
                        if (e->var->getType() == VOID)
                        {
                            UI_debug_pile(" Adding a variable\n");
                            
                            bool isFn = false;
                            list<Token>::iterator g = e;
                            g++;
                            if(g != tokens.end())
                            {
                                if(g->type == Token::SEPARATOR && g->sep == OPEN_PARENTHESIS)
                                    isFn = true;
                            }
                            
                            
                            
                            if(isFn)
                            {
                                Function* f = new Function(e->text, FN_NONE);
                                f->lineNumber = lineNumber;
                                f->returnType = newType;
                                f->reference = true;
                                
                                
                                
                                string name = e->text;
                                
                                // Read signature: void fn(int a, bool b)
                                e++;
                                if(e == tokens.end() || e->type != Token::SEPARATOR || e->sep != OPEN_PARENTHESIS)
                                {
                                    error("Error: Expected open parenthesis.\n");
                                    break;
                                }
                                // void fn(
                                e++;
                                if(e == tokens.end())
                                {
                                    error("Error: Expected parameter list or closing parenthesis.\n");
                                    break;
                                }
                                UI_debug_pile("Loading function declaration.\n");
                                TypeName* gotType = NULL;
                                while(e != tokens.end())
                                {
                                    UI_debug_pile("Checking token: %s\n", e->text.c_str());
                                    if(gotType == NULL)
                                    {
                                        // Find type name, comma, or close paren.
                                        if(e->type == Token::VARIABLE)
                                        {
                                            assert(e->var != NULL);
                                            if(e->var->getType() == TYPENAME)
                                            {
                                                gotType = dynamic_cast<TypeName*>(e->var);
                                                UI_debug_pile("Got type\n");
                                                // Check if it's a reference
                                                list<Token>::iterator g = e;
                                                g++;
                                                if(g != tokens.end() && g->type == Token::OPERATOR && g->oper == BITWISE_AND)
                                                {
                                                    gotType->reference = true;
                                                    e++;
                                                }
                                            }
                                            else
                                            {
                                                error("Error: Unexpected token in function declaration.\n");
                                                //return Token();
                                                break;
                                            }
                                        }
                                        else if(e->type == Token::SEPARATOR)
                                        {
                                            if(e->sep == CLOSE_PARENTHESIS)
                                            {
                                                UI_debug_pile("Done with parameters.\n");
                                                break;
                                            }
                                            else if(e->sep == COMMA)
                                            {
                                                UI_debug_pile("Got comma\n");
                                                gotType = NULL;
                                            }
                                            else
                                            {
                                                error("Error: Unexpected separator in function declaration.\n");
                                                //return Token();
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            error("Error: Unexpected token in function declaration.\n");
                                            //return Token();
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        // Find variable name
                                        if(e->type == Token::VARIABLE)
                                        {
                                            assert(e->var != NULL);
                                            if(e->var->getType() != TYPENAME && e->var->getType() != NOT_A_TYPE && e->var->getType() != CLASS_OBJECT)
                                            {
                                                // Add the variable to the function
                                                f->addArg(*gotType, e->text);
                                                gotType = NULL;
                                                UI_debug_pile("Got parameter.\n");
                                            }
                                            else
                                            {
                                                error("Error: Unexpected variable in function declaration.\n");
                                                //return Token();
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            error("Error: Unexpected token in function declaration.\n");
                                            //return Token();
                                            break;
                                        }
                                    }
                                    e++;
                                }
                                
                                UI_debug_pile("Adding function declaration.\n");
                                
                                e++;
                                if(e != tokens.end())
                                {
                                    error("Error: Unexpected tokens after function declaration.\n");
                                }
                                
                                //addVar(name, f);
                                //return firstVar;
                                
                                defineFunction(f, stream);
                                theClass->addFunction(name, f);
                                break;
                            }
                            theClass->addVariable(getTypeString(newType), e->text);
                            e++;
                            if(e != tokens.end())
                            {
                                error("Error: Unexpected tokens after variable declaration.\n");
                            }
                        }
                        else
                        {
                            error("Error: Unexpected token after type name.\n");
                        }
                    }
                    break;
                }
                e++;
            }
            //returnValue = interpreter.evalTokens(tokens, true, wasTrueIf, wasFalseIf);
            /*if(returnValue.type == Token::VARIABLE)
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
            }*/
            lineNumber++;
        }
        else if(continuation)
            lineNumber++;  // Skip an extra line if the last one was continued...  This probably isn't right for multiple-line continues...
    }
    
    return true;
}


/*
Token evalIf(list<Token>::iterator& e, list<Token>& tokens, bool beginning, bool wasTrueIf, bool wasFalseIf, bool subExpression)
{
    e++;
    if(e == tokens.end())
    {
        error("Syntax Error: Unexpected end of line after 'if'.\n");
        return Token();
    }
    if(e->type != Token::SEPARATOR || e->sep != OPEN_PARENTHESIS)
    {
        error("Syntax Error: Unexpected token after 'if'.\n");
        return Token();
    }
    
    list<Token> inside;
    e++;
    int paren = 1;
    while(e != tokens.end() && paren > 0)
    {
        if(e->type == Token::SEPARATOR)
        {
            if(e->sep == OPEN_PARENTHESIS)
                paren++;
            else if(e->sep == CLOSE_PARENTHESIS)
                paren--;
        }
        
        if(paren > 0)
            inside.push_back(*e);
        e++;
    }
    
    Token in = evalTokens(inside, false, false, false, true);
    
    if(in.type == Token::VARIABLE && in.var != NULL)
    {
        // FIXME: Accept other types too!
        if(in.var->getType() != BOOL)
        {
            in.var = boolCast(in.var);
            if(in.var == NULL)
            {
                error("Syntax Error: Could not cast expression to 'bool' in your 'if' statement.\n");
                return Token();
            }
        }
        
        if(static_cast<Bool*>(in.var)->getValue())
        {
            // Push a single-line scope that evals the 'if' block
            pushEnv(Scope(false, Scope::IF_BLOCK, true));
            return Token(Token::KEYWORD, "true if");
        }
        else
        {
            // Push a single-line scope that skips the 'if' block
            pushEnv(Scope(false, Scope::SKIP_IF, true));
            return Token(Token::KEYWORD, "false if");
        }
    }
    
    error("Syntax Error: Something's wrong with your 'if' statement.\n");
    return Token();
}*/



/*
Evaluates the tokens for a single line.
*/
EvalState Interpreter::evalTokens(list<Token>& tokens, EvalState e_state)
{
    enum StateEnum{BEGIN, READY, VAR_DECL, VAR, VAR_OP, VAR_OP_VAR};
    StateEnum state;
    if(e_state.hasFlag(EvalState::BEGINNING))
        state = BEGIN;
    else
        state = READY;
    UI_debug_pile("Evaluating tokens: ");
    for(list<Token>::iterator e = tokens.begin(); e != tokens.end(); e++)
    {
        if(e->type == Token::OPERATOR)
            UI_debug_pile("['%s' '%s'] ", "Operator", getOperatorString(e->oper).c_str());
        else if(e->var != NULL)
            UI_debug_pile("['%s' (%s)] ", e->var->getTypeString().c_str(), e->var->getValueString().c_str());
    }
    UI_debug_pile("\n");
    TypeName* newTypeName = NULL;  // Used for Variable declarations
    TypeEnum newType = NOT_A_TYPE;  // Used for Variable declarations
    Token firstVar;
    Token firstOp;
    Token secondVar;
    vector<list<Token> > stack;  // Used to hold tokens when considering precedence.
    stack.push_back(list<Token>());
    // The nested Vector is for parenthesized expressions.
    bool closingExpression = false;  // When we hit the end of the list or a close paren, 
                                     // then we use this.

    // Find a Variable, find an operator, find a Variable, find an operator.
    // If the second operator has lower (earlier) precedence, then push the
    // first Variable and operator and keep going.  If the first operator has
    // precedence, then evaluate it.  If the two have the same precedence and
    // the associativity is right-to-left, then push the first.
    // When an open-parenthesis is hit, push the first Variable and operator, then
    // push a new vector<> to the stack.  When a close-parenthesis is hit,
    // evaluate the top vector, then pop it.
    for (list<Token>::iterator e = tokens.begin(); e != tokens.end();)
    {
        if(e->type == Token::OPERATOR)
            UI_debug_pile(" Token name: %s\n", getOperatorString(e->oper).c_str());
        else
            UI_debug_pile(" Token name: %s\n", e->text.c_str());
        
        if(currentScope().state == Scope::SKIP_IF)
        {
            // If there's an open bracket, we would change to multi-line scope.
            // FIXME: Doesn't handle matching brackets to ignore nested bracket sets
            // If there's a close bracket in multi-line mode, we end the skipping
            
            bool ignoreIt = (e->type != Token::SEPARATOR || (e->sep != OPEN_CURLY_BRACKET && e->sep != CLOSE_CURLY_BRACKET));
            
            if(ignoreIt)
            {
                // If it's a single line, then pop the scope
                if(currentScope().singleLine)
                {
                    popEnv();
                    return Token(Token::KEYWORD, "false if");
                }
                // FIXME: Doesn't see a bracket on a non-empty line
                // Skip the line
                return Token();
            }
            else
            {
                if(e->sep == OPEN_CURLY_BRACKET)
                {
                    if(currentScope().singleLine)
                    {
                        // Change scope to multi-line
                        currentScope().singleLine = false;
                    }
                    else
                    {
                        // FIXME: Increment count of the open brackets
                    }
                    //state = BEGIN;
                    // FIXME: Doesn't see anything else on a bracket line
                    // Skip the rest of the line
                    return Token();
                }
                else if(e->sep == CLOSE_CURLY_BRACKET)
                {
                    if(currentScope().singleLine)
                    {
                        error("Syntax Error: Unexpected close bracket in single-line 'if' statement.\n");
                    }
                    else
                    {
                        // FIXME: Decrement count of the open brackets
                        popEnv();
                        return Token(Token::KEYWORD, "false if");
                    }
                    //state = BEGIN;
                    // FIXME: Doesn't see anything else on a bracket line
                    // Skip the rest of the line
                    return Token();
                }
            }
        }
        
        if (e->type == Token::VARIABLE)
        {
            if (state == BEGIN)
            {
                // Declaring new variables...
                // "TypeName variableName"
                //  (new type) (VOID)
                if (e->var->getType() == TYPENAME)
                {
                    if(!allowDeclarations)
                    {
                        error("Variable declarations are disabled in this file.\n");
                        return Token();
                    }
                    TypeName* t = static_cast<TypeName*>(e->var);
                    t->text = e->text;
                    newTypeName = t;
                    newType = t->getValue();
                    UI_debug_pile(" Found a type name\n");
                    state = VAR_DECL;
                    if(newType == ARRAY)
                    {
                        // FIXME: Make this recursible
                        // Figure out what value type is in the array
                        Token left, inside, right;
                        list<Token>::iterator g = e;
                        g++;
                        if(g != tokens.end())
                        {
                            left = *g;
                            g++;
                            if(g != tokens.end())
                            {
                                inside = *g;
                                g++;
                                if(g != tokens.end())
                                {
                                    right = *g;
                                    // Got all three!
                                    if(left.type == Token::OPERATOR && left.oper == LESS
                                       && right.type == Token::OPERATOR && right.oper == GREATER
                                       && inside.type == Token::VARIABLE && inside.var != NULL)
                                    {
                                        e = g;
                                        t->subType = getTypeFromString(inside.text);
                                    }
                                }
                            }
                        }
                    }
                }
                else if (e->var->getType() == CLASS)
                {
                    TypeName* t = new TypeName(e->text, CLASS_OBJECT);
                    t->text = e->text;
                    newTypeName = t;
                    newType = CLASS_OBJECT;
                    UI_debug_pile(" Found a class name\n");
                    state = VAR_DECL;
                }
                else if (e->var->getType() != VOID && e->var->getType() != NOT_A_TYPE)
                {
                    UI_debug_pile("Found a variable at beginning.\n");
                    firstVar = *e;
                    state = VAR;
                }
                else
                {
                    error("Syntax Error: Variable '%s' is not declared.\n", e->text.c_str());
                    UI_debug_pile("Variable '%s'.  Type '%s'.\n", e->text.c_str(), e->var->getTypeString().c_str());
                    UI_debug_pile("Num scopes: %d\n", env.size());
                }
            }
            else if(state == READY)
            {
                if (e->var->getType() == TYPENAME)
                {
                    error("Syntax Error: Type '%s' can not be declared here.\n", e->text.c_str());
                }
                else if (e->var->getType() != VOID && e->var->getType() != NOT_A_TYPE)
                {
                    firstVar = *e;
                    state = VAR;
                }
                else
                {
                    error("Syntax Error: Variable '%s' is not declared.\n", e->text.c_str());
                    UI_debug_pile("Variable '%s'.  Type '%s'.\n", e->text.c_str(), e->var->getTypeString().c_str());
                    UI_debug_pile("Num scopes: %d\n", env.size());
                }
            }
            else if (state == VAR_DECL)
            {
                // Declaring new variables...
                if (e->var->getType() == VOID)
                {
                    UI_debug_pile(" Adding a variable\n");
                    Variable* v = NULL;
                    bool isFn = false;
                    list<Token>::iterator g = e;
                    g++;
                    if(g != tokens.end())
                    {
                        if(g->type == Token::SEPARATOR && g->sep == OPEN_PARENTHESIS)
                            isFn = true;
                    }
                    
                        
                    string name = e->text;
                    
                    
                    if(isFn)
                    {
                        Function* f = new Function(e->text, FN_NONE);
                        f->lineNumber = lineNumber;
                        f->returnType = newType;
                        f->reference = true;
                        
                        
                        delete e->var;
                        e->var = f;
                        
                        firstVar = *e;
                        
                        // Read signature: void fn(int a, bool b)
                        e++;
                        if(e == tokens.end() || e->type != Token::SEPARATOR || e->sep != OPEN_PARENTHESIS)
                        {
                            error("Error: Expected open parenthesis.\n");
                            return Token();
                        }
                        // void fn(
                        e++;
                        if(e == tokens.end())
                        {
                            error("Error: Expected parameter list or closing parenthesis.\n");
                            return Token();
                        }
                        UI_debug_pile("Loading function declaration.\n");
                        TypeName* gotType = NULL;
                        while(e != tokens.end())
                        {
                            UI_debug_pile("Checking token: %s\n", e->text.c_str());
                            if(gotType == NULL)
                            {
                                // Find type name, comma, or close paren.
                                if(e->type == Token::VARIABLE)
                                {
                                    assert(e->var != NULL);
                                    if(e->var->getType() == TYPENAME)
                                    {
                                        gotType = dynamic_cast<TypeName*>(e->var);
                                        UI_debug_pile("Got type\n");
                                        
                                        // Check if it's a reference
                                        list<Token>::iterator g = e;
                                        g++;
                                        if(g != tokens.end() && g->type == Token::OPERATOR && g->oper == BITWISE_AND)
                                        {
                                            gotType->reference = true;
                                            e++;
                                        }
                                    }
                                    else
                                    {
                                        error("Error: Unexpected token in function declaration.\n");
                                        return Token();
                                    }
                                }
                                else if(e->type == Token::SEPARATOR)
                                {
                                    if(e->sep == CLOSE_PARENTHESIS)
                                    {
                                        UI_debug_pile("Done with parameters.\n");
                                        break;
                                    }
                                    else if(e->sep == COMMA)
                                    {
                                        UI_debug_pile("Got comma\n");
                                        gotType = NULL;
                                    }
                                    else
                                    {
                                        error("Error: Unexpected separator in function declaration.\n");
                                        return Token();
                                    }
                                }
                                else
                                {
                                    error("Error: Unexpected token in function declaration.\n");
                                    return Token();
                                }
                            }
                            else
                            {
                                // Find variable name
                                if(e->type == Token::VARIABLE)
                                {
                                    assert(e->var != NULL);
                                    if(e->var->getType() != TYPENAME && e->var->getType() != NOT_A_TYPE && e->var->getType() != CLASS_OBJECT)
                                    {
                                        // Add the variable to the function
                                        f->addArg(*gotType, e->text);
                                        gotType = NULL;
                                        UI_debug_pile("Got parameter.\n");
                                    }
                                    else
                                    {
                                        error("Error: Unexpected variable in function declaration.\n");
                                        return Token();
                                    }
                                }
                                else
                                {
                                    error("Error: Unexpected token in function declaration.\n");
                                    return Token();
                                }
                            }
                            e++;
                        }
                        
                        UI_debug_pile("Adding function declaration.\n");
                        addVar(name, f);
                        
                        return firstVar;
                    }
                    
                    // Set the new variable
                    if (newType == STRING)
                        v = new String(name, "");
                    else if (newType == BOOL)
                        v = new Bool(name, false);
                    else if (newType == INT)
                        v = new Int(name, 0);
                    else if (newType == FLOAT)
                        v = new Float(name, 0.0f);
                    else if (newType == MACRO)
                        v = new Macro(name);
                    else if (newType == ARRAY)
                    {
                        Array* a = new Array(name, NOT_A_TYPE);
                        if(newTypeName != NULL && newTypeName->subType != NOT_A_TYPE)
                        {
                            a->setValueType(newTypeName->subType);
                        }
                        v = a;
                    }
                    else if (newType == LIST)
                        v = new List(name);
                    else if (newType == FUNCTION)
                        v = new Function(name, FN_NONE);
                    else if (newType == PROCEDURE)
                        v = new Procedure(name);
                    else if (newType == CLASS)
                    {
                        Class* c = new Class(name);
                        v = c;
                    }
                    else if (newType == CLASS_OBJECT)
                    {
                        ClassObject* c = new ClassObject(name, newTypeName->text);
                        if(c->className == "")
                            error("Error: Unknown class, %s, for variable '%s'\n", newTypeName->text.c_str(), e->text.c_str());
                        v = c;
                    }
                    else if (newType == VOID)
                    {
                        error("Error: Void type for variable '%s'\n");
                    }
                    else
                    {
                        error("Error: Unknown type for variable '%s'\n", e->text.c_str());
                    }
                    
                    if(!e->var->literal)
                        v->reference = true;
                    
                    addVar(e->text, v);
                    delete e->var;
                    e->var = v;

                    newType = NOT_A_TYPE;
                    firstVar = *e;
                    state = VAR;
                }
                else  // Bad syntax
                {
                    error("Error: Multiple declarations of variable '%s'\n", e->text.c_str());
                }
            }
            else if (state == VAR)
            {
                error("Syntax Error1: Unexpected variable (%s).  Expected an operator or parenthesis.\n", e->text.c_str());
            }
            else if (state == VAR_OP)
            {
                secondVar = *e;
                state = VAR_OP_VAR;
            }
            else if(state == VAR_OP_VAR)
            {
                error("Syntax Error2: Unexpected variable.  Expected an operator or parenthesis.\n");
            }
        }
        else if(e->type == Token::OPERATOR)
        {
            if (state == BEGIN)
            {
                if(e->oper == SUBTRACT)
                {
                    
                }
                else if(e->oper == NOT)
                {
                    
                }
                else
                {
                    error("Syntax Error: Unexpected binary operator at the beginning of a line.\n");
                }
            }
            else if(state == READY)
            {
                if(e->oper == NEGATE)
                {
                    
                }
                else if(e->oper == NOT)
                {
                    
                }
                else
                {
                    error("Syntax Error: Unexpected binary operator at the beginning of an expression.\n");
                }
            }
            else if (state == VAR_DECL)
            {
                error("Syntax Error: Unexpected operator.  Expected a variable to be declared.\n");
            }
            else if (state == VAR)
            {
                firstOp = *e;
                state = VAR_OP;
            }
            else if (state == VAR_OP)
            {
                error("Syntax Error: Unexpected operator.  Expected a variable or parenthesis.\n");
            }
            else if(state == VAR_OP_VAR)
            {
                // Compare operators
                if(e->precedence < firstOp.precedence // New one goes earlier
                   || (e->precedence == firstOp.precedence && firstOp.associativeLeftToRight == false))  // Right-to-left assoc.
                {
                    // Push our old stuff
                    stack[0].push_front(firstVar);
                    stack[0].push_front(firstOp);
                    firstVar = secondVar;
                    firstOp = *e;
                    state = VAR_OP;
                }
                else  // Evaluate the first operator
                {
                    firstVar = Token(evaluateExpression(firstVar.var, firstOp.oper, secondVar.var), "");
                    firstOp = *e;
                    state = VAR_OP;
                }
            }
        }
        else if(e->type == Token::SEPARATOR)
        {
            if (state == BEGIN)
            {
                if(e->sep == OPEN_PARENTHESIS)
                {
                    // Move elements between the parentheses into a new list to
                    // be evaluated.
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_PARENTHESIS)
                                num++;
                            else if(e->sep == CLOSE_PARENTHESIS)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    EvalState es = evalTokens(tok2, EvalState(EvalState::SUBEXPRESSION));
                    if(es.state & EvalState::ERROR)
                        return es;
                    firstVar = es.token;
                    if(firstVar.type == Token::NOT_A_TOKEN)
                    {
                        return firstVar;
                    }
                    state = VAR;
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis at the beginning of a line.\n");
                }
                else if(e->sep == OPEN_CURLY_BRACKET)
                {
                    if(currentScope().singleLine)
                        currentScope().singleLine = false;
                    else
                    {
                        // Push new scope
                        pushEnv(false);
                    }
                }
                else if(e->sep == CLOSE_CURLY_BRACKET)
                {
                    if(currentScope().singleLine)
                        error("Syntax Error: Unexpected closing curly bracket at the beginning of a line.\n");
                    else
                    {
                        popEnv();
                        return Token(Token::KEYWORD, "true if");
                    }
                }
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    EvalState es = getArrayLiteral(tokens, e);
                    if(es.state & EvalState::ERROR)
                        return es;
                    firstVar = es.token;
                    if(firstVar.var != NULL)
                    {
                        state = VAR;
                    }
                }
                else
                {
                    error("Syntax Error: Unexpected separator at the beginning of a line.\n");
                }
            }
            else if(state == READY)
            {
                if(e->sep == OPEN_PARENTHESIS)
                {
                    // Move elements between the parentheses into a new list to
                    // be evaluated.
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_PARENTHESIS)
                                num++;
                            else if(e->sep == CLOSE_PARENTHESIS)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    EvalState es = evalTokens(tok2, EvalState(EvalState::SUBEXPRESSION));
                    if(es.state & EvalState::ERROR)
                        return es;
                    firstVar = es.token;
                    if(firstVar.type == Token::NOT_A_TOKEN)
                    {
                        return firstVar;
                    }
                    state = VAR;
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis at the beginning of an expression.\n");
                }
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    EvalState es = getArrayLiteral(tokens, e);
                    if(es.state & EvalState::ERROR)
                        return es;
                    
                    firstVar = es.token;
                    if(firstVar.var != NULL)
                    {
                        state = VAR;
                    }
                }
                else
                {
                    error("Syntax Error: Unexpected separator at the beginning of an expression.\n");
                }
            }
            else if (state == VAR_DECL)
            {
                error("Syntax Error: Unexpected separator.  Expected a variable to be declared.\n");
            }
            else if (state == VAR)
            {
                if(e->sep == OPEN_PARENTHESIS)
                {
                    // VAR() -> Function call!
                    
                    // Find the argument tokens "arg1, arg2, arg3, ..."
                    
                    // Put all the tokens between matching parentheses into a new list
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_PARENTHESIS)
                                num++;
                            else if(e->sep == CLOSE_PARENTHESIS)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    
                    UI_debug_pile("Just before function call.\n");
                    
                    // Push the function stack
                    stack.push_back(list<Token>());
                    
                    // Call the function
                    if(firstVar.var->getType() == FUNCTION)
                    {
                        EvalState es = callFn(static_cast<Function*>(firstVar.var), tok2);
                        if(es.state & EvalState::ERROR)
                            return es;
                        firstVar = es.token;
                    }
                    
                    // Pop the outer stack
                    vector<list<Token> >::iterator g = stack.end();
                    g--;
                    stack.erase(g);
                    
                    UI_debug_pile("Just finished function call.\n");
                    
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis.\n");
                }
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    // Evaluate the contents of the brackets
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_SQUARE_BRACKET)
                                num++;
                            else if(e->sep == CLOSE_SQUARE_BRACKET)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    EvalState es = evalTokens(tok2, EvalState(EvalState::SUBEXPRESSION));
                    if(es.state & EvalState::ERROR)
                        return es;
                    Token r = es.token;
                    if(r.type == Token::NOT_A_TOKEN)
                    {
                        return r;
                    }
                    
                    firstVar = Token(evaluateExpression(firstVar.var, ARRAY_ACCESS, r.var), "");
                    state = VAR;
                }
                else
                {
                    error("Syntax Error: Unexpected separator.\n");
                }
            }
            else if (state == VAR_OP)
            {
                if(e->sep == OPEN_PARENTHESIS)
                {
                    // Move elements between the parentheses into a new list to
                    // be evaluated.
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_PARENTHESIS)
                                num++;
                            else if(e->sep == CLOSE_PARENTHESIS)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    EvalState es = evalTokens(tok2, EvalState(EvalState::SUBEXPRESSION));
                    if(es.state & EvalState::ERROR)
                        return es;
                    secondVar = es.token;
                    if(secondVar.type == Token::NOT_A_TOKEN)
                    {
                        return secondVar;
                    }
                    state = VAR_OP_VAR;
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis after an operator.  Ignoring the operator...\n");
                    state = VAR;
                }
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    EvalState es = getArrayLiteral(tokens, e);
                    if(es.state & EvalState::ERROR)
                        return es;
                    
                    secondVar = es.token;
                    if(secondVar.var != NULL)
                    {
                        state = VAR_OP_VAR;
                        Array* arr = static_cast<Array*>(secondVar.var);
                        for(unsigned int i = 0; i < arr->getValue().size(); i++)
                        {
                            UI_debug_pile("    Element: %s\n", arr->getValue()[i]->getValueString().c_str());
                        }
                    }
                }
                else
                    error("Syntax Error: Unexpected separator.  Expected a variable or parenthesis.\n");
            }
            else if(state == VAR_OP_VAR)
            {
                if(e->sep == OPEN_PARENTHESIS)
                {
                    // VAR() -> Function call!
                    
                    // If it's a dot operator, we do that first.  This would be
                    // handled by precedence if parens were operators again...
                    if(firstOp.oper == DOT)
                    {
                        secondVar = Token(evaluateExpression(firstVar.var, firstOp.oper, secondVar.var), "");
                        state = VAR;
                    }
                    
                    
                    // Find the argument tokens "arg1, arg2, arg3"
                    
                    // Put all the tokens between matching parentheses into a new list
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_PARENTHESIS)
                                num++;
                            else if(e->sep == CLOSE_PARENTHESIS)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    
                    // Push the function stack
                    stack.push_back(list<Token>());
                    
                    // Call the function
                    if(secondVar.var->getType() == FUNCTION)
                    {
                        EvalState es = callFn(static_cast<Function*>(secondVar.var), tok2);
                        if(es.state & EvalState::ERROR)
                            return es;
                        secondVar.var = es.token.var;
                    }
                    
                    // Pop the outer stack
                    vector<list<Token> >::iterator g = stack.end();
                    g--;
                    stack.erase(g);
                    
                    // If we had a dot operator...
                    if(state == VAR)
                        firstVar = secondVar;
                    
                    if(secondVar.type == Token::NOT_A_TOKEN)
                    {
                        return secondVar;
                    }
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis.\n");
                }
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    // FIXME: Hacky
                    // Compare operators
                    if(2 < firstOp.precedence // New one goes earlier
                       || (2 == firstOp.precedence && firstOp.associativeLeftToRight == false))  // Right-to-left assoc.
                    {
                        // Push our old stuff
                        stack[0].push_front(firstVar);
                        stack[0].push_front(firstOp);
                        firstVar = secondVar;
                    }
                    else  // Evaluate the first operator
                    {
                        firstVar = Token(evaluateExpression(firstVar.var, firstOp.oper, secondVar.var), "");
                    }
                    
                    // Evaluate the contents of the brackets
                    list<Token> tok2;
                    e++;
                    list<Token>::iterator f = e;
                    int num = 1;
                    while(e != tokens.end() && num > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_SQUARE_BRACKET)
                                num++;
                            else if(e->sep == CLOSE_SQUARE_BRACKET)
                                num--;
                        }
                        if(num > 0)
                            e++;
                    }
                    tok2.splice(tok2.begin(), tokens, f, e);
                    EvalState es = evalTokens(tok2, EvalState(EvalState::SUBEXPRESSION));
                    if(es.state & EvalState::ERROR)
                        return es;
                    Token r = es.token;
                    if(r.type == Token::NOT_A_TOKEN)
                    {
                        return r;
                    }
                    
                    firstVar = Token(evaluateExpression(firstVar.var, ARRAY_ACCESS, r.var), "");
                    state = VAR;
                }
                else
                    error("Syntax Error: Unexpected separator.\n");
            }
        }
        else if(e->type == Token::KEYWORD)
        {
            if (state == BEGIN)
            {
                if(e->keyword == KW_IF)
                {
                    e++;
                    if(e == tokens.end())
                    {
                        error("Syntax Error: Unexpected end of line after 'if'.\n");
                        return Token();
                    }
                    if(e->type != Token::SEPARATOR || e->sep != OPEN_PARENTHESIS)
                    {
                        error("Syntax Error: Unexpected token after 'if'.\n");
                        return Token();
                    }
                    
                    list<Token> inside;
                    e++;
                    int paren = 1;
                    while(e != tokens.end() && paren > 0)
                    {
                        if(e->type == Token::SEPARATOR)
                        {
                            if(e->sep == OPEN_PARENTHESIS)
                                paren++;
                            else if(e->sep == CLOSE_PARENTHESIS)
                                paren--;
                        }
                        
                        if(paren > 0)
                            inside.push_back(*e);
                        e++;
                    }
                    
                    EvalState es = evalTokens(inside, EvalState(EvalState::SUBEXPRESSION));
                    if(es.state & EvalState::ERROR)
                        return es;
                    Token in = es.token;
                    
                    if(in.type == Token::VARIABLE && in.var != NULL)
                    {
                        // FIXME: Accept other types too!
                        if(in.var->getType() != BOOL)
                        {
                            in.var = boolCast(in.var);
                            if(in.var == NULL)
                            {
                                error("Syntax Error: Could not cast expression to 'bool' in your 'if' statement.\n");
                                return Token();
                            }
                        }
                        
                        if(static_cast<Bool*>(in.var)->getValue())
                        {
                            // Push a single-line scope that evals the 'if' block
                            pushEnv(Scope(false, Scope::IF_BLOCK, true));
                            return Token(Token::KEYWORD, "true if");
                        }
                        else
                        {
                            // Push a single-line scope that skips the 'if' block
                            pushEnv(Scope(false, Scope::SKIP_IF, true));
                            return Token(Token::KEYWORD, "false if");
                        }
                    }
                    
                    error("Syntax Error: Something's wrong with your 'if' statement.\n");
                    return Token();
                }
                else if(e->keyword == KW_ELSE)
                {
                    e++;
                    if(e != tokens.end())
                    {
                        error("Syntax Error: Unexpected token after 'else' statement.\n");
                    }
                    if(e_state.hasFlag(EvalState::WAS_FALSE_IF))
                        pushEnv(Scope(false, Scope::IF_BLOCK, true));
                    else if(e_state.hasFlag(EvalState::WAS_TRUE_IF))
                        pushEnv(Scope(false, Scope::SKIP_IF, true));
                    else
                        error("Syntax Error: Unexpected 'else' statement.\n");
                    return Token(Token::KEYWORD, "else");
                }
                else if(e->keyword == KW_RETURN)
                {
                    state = READY;
                    stack[0].push_front(Token(Token::KEYWORD, "return"));
                }
                else
                {
                    error("Syntax Error: Unexpected keyword at the beginning of a line.\n");
                }
            }
            else if(state == READY)
            {
                if(false)  // Put keywords here that can be used at the start of any expression
                    ;
                else
                {
                    error("Syntax Error: Unexpected keyword at the beginning of an expression.\n");
                }
            }
            else if (state == VAR_DECL)
            {
                error("Syntax Error: Unexpected keyword.  Expected a variable to be declared.\n");
            }
            else if (state == VAR)
            {
                error("Syntax Error1: Unexpected keyword.  Expected an operator or parenthesis.\n");
            }
            else if (state == VAR_OP)
            {
                error("Syntax Error: Unexpected keyword.  Expected a variable or parenthesis.\n");
            }
            else if(state == VAR_OP_VAR)
            {
                error("Syntax Error2: Unexpected keyword.  Expected an operator or parenthesis.\n");
            }
        }
        else
        {
            if (state == BEGIN)
            {
                error("Syntax Error: Unexpected token at the beginning of a line.\n");
            }
            else if(state == READY)
            {
                error("Syntax Error: Unexpected token at the beginning of an expression.\n");
            }
            else if (state == VAR_DECL)
            {
                error("Syntax Error: Unexpected token.  Expected a variable to be declared.\n");
            }
            else if (state == VAR)
            {
                error("Syntax Error1: Unexpected token.  Expected an operator or parenthesis.\n");
            }
            else if (state == VAR_OP)
            {
                error("Syntax Error: Unexpected token.  Expected a variable or parenthesis.\n");
            }
            else if(state == VAR_OP_VAR)
            {
                error("Syntax Error2: Unexpected token.  Expected an operator or parenthesis.\n");
            }
        }
          
        
        
        // Get next token...
        // This will also skip the closing parenthesis if it's there.
        if(e != tokens.end())
            e++;
        
        if(e == tokens.end())
        {
            // Does this happen in parentheses?  No...  I should push the parenthesis onto the new stack.
            if(stack[0].size() == 0)
            {
                // Wrap up the collected tokens
                if(state == VAR_OP_VAR)
                {
                    firstVar = Token(evaluateExpression(firstVar.var, firstOp.oper, secondVar.var), "");
                }
                else if(state == VAR)
                {
                    // Just a variable left...  We should be done.
                }
                else if(state != BEGIN)
                {
                    error("Error: Malformed line\n");
                }
                
                
                // All done
                break;
            }
            
            // Push everything onto the stack
            if(state == VAR || state == VAR_OP || state == VAR_OP_VAR)
            {
                stack[0].push_front(firstVar);
                if(state == VAR_OP || state == VAR_OP_VAR)
                {
                    stack[0].push_front(firstOp);
                    if(state == VAR_OP_VAR)
                    {
                        stack[0].push_front(secondVar);
                    }
                }
            }
            
            closingExpression = true;
            state = READY;
        }
        
        
        if(closingExpression)
        {
            UI_debug_pile("Closing expression...\n");
            // Loop over the stack until it's empty
            while(stack[0].size() > 0)
            {
                UI_debug_pile("Looping back...\n");
                // Pop the inner stack (from the front)
                Token prev = *(stack[0].begin());
                stack[0].erase(stack[0].begin());
                
                
                if(prev.type == Token::OPERATOR)
                    UI_debug_pile(" Retrieved: Operator '%s'\n", getOperatorString(prev.oper).c_str());
                else if(prev.type == Token::SEPARATOR)
                    UI_debug_pile(" Retrieved: Separator '%s'\n", getSeparatorString(prev.sep).c_str());
                else
                    UI_debug_pile(" Retrieved: %s\n", prev.text.c_str());
                /*if(prev->type == Token::OPERATOR && prev->oper == OPEN_PARENTHESIS)
                {
                    // Pop the outer stack (from the back)
                    if(stack.size() > 1)
                    {
                        vector<list<Token*> >::iterator s = stack.end();
                        s--;
                        stack.erase(s);
                    }
                    else
                    {
                        // Error...  but really shouldn't get here.
                    }
                    closingExpression = false;
                    continue;
                }*/
                if(state == READY)
                {
                    if(prev.type == Token::OPERATOR)
                    {
                        // This shouldn't happen
                        error("Error: Unexpected operator when evaluating expression.\n");
                    }
                    else if(prev.type == Token::SEPARATOR)
                    {
                        // This shouldn't happen
                        error("Error: Unexpected separator when evaluating expression.\n");
                    }
                    else if(prev.type == Token::KEYWORD)
                    {
                        if(prev.keyword == KW_RETURN)
                        {
                            return Token(Token::KEYWORD, "return");
                        }
                    }
                    else
                    {
                        secondVar = prev;
                        state = VAR;
                    }
                }
                else if (state == VAR)
                {
                    if (prev.type == Token::SEPARATOR)
                    {
                        /*if(prev->sep == OPEN_PARENTHESIS)
                        {
                            // Function call goes here...
                            
                            stack[0].push_front(firstVar);
                            stack[0].push_front(new Token("*"));  // Implied multiplication
                            stack.push_back(list<Token*>());
                            state = READY;
                        }
                        else if(prev->sep == CLOSE_PARENTHESIS)
                        {
                            if(stack.size() > 1)
                            {
                                closingExpression = true;
                            }
                            else
                            {
                                error("Syntax Error: Unexpected closing parenthesis.\n");
                            }
                        }
                        else
                        {
                            firstOp = prev;
                            state = VAR_OP;
                        }*/
                        
                        error("Syntax Error3: Unexpected separator.  Expected an operator or parenthesis.\n");
                    }
                    else if(prev.type == Token::OPERATOR)
                    {
                        firstOp = prev;
                        state = VAR_OP;
                    }
                    else if(prev.type == Token::KEYWORD)
                    {
                        if(prev.keyword == KW_RETURN)
                        {
                            Token t = Token(Token::KEYWORD, "return");
                            t.var = secondVar.var;
                            return t;
                        }
                    }
                    else
                    {
                        error("Syntax Error3: Unexpected token.  Expected an operator or parenthesis.\n");
                    }
                }
                else if (state == VAR_OP)
                {
                    if (prev.type == Token::VARIABLE)
                    {
                        UI_debug_pile("Evaluating expression: %s %s %s\n", prev.text.c_str(), getOperatorString(firstOp.oper).c_str(), secondVar.text.c_str());
                        secondVar = Token(evaluateExpression(prev.var, firstOp.oper, secondVar.var), "<temp>");
                        
                        // FIXME: Hacky.  The secondVar is not returned correctly.
                        firstVar = secondVar;
                        
                        state = VAR;
                    }
                    else
                    {
                        /*if(e->type == Token::SEPARATOR && e->sep == OPEN_PARENTHESIS)
                        {
                            stack[0].push_front(firstVar);
                            stack[0].push_front(firstOp);
                            stack.push_back(list<Token*>());
                            state = READY;
                        }
                        if(e->sep == Token::OPERATOR && e->sep == CLOSE_PARENTHESIS)
                        {
                            closingExpression = true;
                            
                            error("Syntax Error: Unexpected closing parenthesis after an operator.  Ignoring the operator...\n");
                            state = VAR;
                        }
                        else*/
                            error("Syntax Error: Unexpected token.  Expected a variable or parenthesis.\n");
                    }
                }
            }
            
            // All done.  Now we make sure the return is alright.
            if(state == READY)
            {
                // e.g. "()"
                error("Error: Empty expression.  Using '1' instead.\n");
                secondVar = Token(new Int("<temp>", 1), "<temp>");
            }
            else if(state == VAR)
            {
                // e.g. (2)
                // Nothing to do here.
            }
            else if(state == VAR_OP)
            {
                // e.g. "(+ 1)"
                error("Error: Expression begins with a binary operator.  Ignoring the operator...\n");
            }
            
            
            if(stack.size() > 1)
            {
                // Pop the outer stack and keep going
                vector<list<Token> >::iterator f = stack.end();
                f--;
                stack.erase(f);
                closingExpression = false;
                // Go back to checking new tokens
                // Everything had been pushed, so it could look like:
                // [4 + 3 *](returnValue)[+ 1 + 2]
                // [] are stacks or lists, () is the expression
                // I'll need to get the 3 * back so I can proceed.
                // secondVar is already set to the returnValue.
                
                if(stack[0].size() > 1)
                {
                    firstOp = *(stack[0].begin());
                    stack[0].erase(stack[0].begin());
                    firstVar = *(stack[0].begin());
                    stack[0].erase(stack[0].begin());
                    state = VAR_OP_VAR;
                }
                else
                {
                    // Either something is wrong or there was a nested expression at the beginning.
                    firstVar = secondVar;
                    state = VAR;
                }
            }
            else
            {
                // This was the last stack, so we're done!
                // tokens.size() should be 0
                break;
            }
        }
        
    }
    
    if(!e_state.hasFlag(EvalState::SUBEXPRESSION) && currentScope().singleLine)
    {
        popEnv();
        if(e_state.hasFlag(EvalState::WAS_TRUE_IF))
            return Token(Token::KEYWORD, "true if");
        else if(e_state.hasFlag(EvalState::WAS_FALSE_IF))
            return Token(Token::KEYWORD, "false if");
        else
            return Token();
    }
    
    
    UI_debug_pile("Done evaluation");
    if(firstVar.type == Token::OPERATOR)
        UI_debug_pile(": '%s' (%s)", "Operator", getOperatorString(firstVar.oper).c_str());
    else if(firstVar.type == Token::SEPARATOR)
        UI_debug_pile(": '%s' (%s)", "Separator", getSeparatorString(firstVar.sep).c_str());
    else if(firstVar.var != NULL)
        UI_debug_pile(": '%s' (%s)", firstVar.var->getTypeString().c_str(), firstVar.var->getValueString().c_str());
    else
        UI_debug_pile(": NULL var");
    UI_debug_pile("\n");
    return firstVar;
}

