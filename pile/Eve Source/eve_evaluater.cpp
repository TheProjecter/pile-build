#include "eve_interpreter.h"

#include <list>
using namespace std;

// Evaluates the tokens for a single line.
Token Interpreter::evalTokens(list<Token>& tokens, bool beginning)
{
    enum StateEnum{BEGIN, READY, VAR_DECL, VAR, VAR_OP, VAR_OP_VAR};
    StateEnum state;
    if(beginning)
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
        
        if (state == BEGIN)
        {
            if (e->type == Token::VARIABLE)
            {
                // Declaring new variables...
                // "TypeName variableName"
                //  (new type) (VOID)
                if (e->var->getType() == TYPENAME) // Should probably be an operator...
                {
                    TypeName* t = static_cast<TypeName*>(e->var);
                    newType = t->getValue();
                    UI_debug_pile(" Found a type name\n");
                    state = VAR_DECL;
                }
                else if (e->var->getType() != VOID && e->var->getType() != NOT_A_TYPE)
                {
                    firstVar = *e;
                    state = VAR;
                }
                else
                {
                    error("Syntax Error: Variable '%s' is not declared.\n", e->text.c_str());
                }
            }
            else if(e->type == Token::OPERATOR)
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
            else if(e->type == Token::SEPARATOR)
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
                    firstVar = evalTokens(tok2, false);
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
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    firstVar = Token(getArrayLiteral(tokens, e), "");
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
            else if(e->type == Token::KEYWORD)
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
                    
                    Token in = evalTokens(inside, false);
                    
                    if(in.type == Token::VARIABLE && in.var != NULL)
                    {
                        // FIXME: Accept other types too!
                        if(in.var->getType() == BOOL)
                        {
                            if(static_cast<Bool*>(in.var)->getValue())
                                return Token(Token::KEYWORD, "if");
                            else
                                return Token(Token::KEYWORD, "false if");
                        }
                    }
                    
                    error("Syntax Error: Something's wrong with your 'if' statement.\n");
                    return Token();
                }
                else
                {
                    error("Syntax Error: Unexpected keyword at the beginning of a line.\n");
                }
            }
            else
            {
                error("Syntax Error: Unexpected token at the beginning of a line.\n");
            }
        }
        else if(state == READY)
        {
            if (e->type == Token::VARIABLE)
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
                }
            }
            else if(e->type == Token::OPERATOR)
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
            else if(e->type == Token::SEPARATOR)
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
                    firstVar = evalTokens(tok2, false);
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
                    firstVar = Token(getArrayLiteral(tokens, e), "");
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
            else if(e->type == Token::KEYWORD)
            {
                if(false)  // Put keywords here that can be used at the start of any expression
                    ;
                else
                {
                    error("Syntax Error: Unexpected keyword at the beginning of an expression.\n");
                }
            }
            else
            {
                error("Syntax Error: Unexpected token at the beginning of an expression.\n");
            }
        }
        else if (state == VAR_DECL)
        {
            if (e->type == Token::VARIABLE)
            {
                // Declaring new variables...
                if (e->var->getType() == VOID)
                {
                    UI_debug_pile(" Adding a variable\n");
                    Variable* v = NULL;
                    // Set the new variable
                    if (newType == STRING)
                        v = new String;
                    else if (newType == BOOL)
                        v = new Bool;
                    else if (newType == INT)
                        v = new Int;
                    else if (newType == FLOAT)
                        v = new Float;
                    else if (newType == MACRO)
                        v = new Macro;
                    else if (newType == ARRAY)
                        v = new Array;
                    else if (newType == LIST)
                        v = new List;
                    else if (newType == FUNCTION)
                        v = new Function;
                    else if (newType == PROCEDURE)
                        v = new Procedure;
                    else
                    {
                        error("Error: Unknown type for variable '%s'\n", e->text.c_str());
                    }
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
            else
            {
                error("Syntax Error: Unexpected token.  Expected a variable to be declared.\n");
            }
        }
        else if (state == VAR)
        {
            if (e->type == Token::SEPARATOR)
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
                    
                    // Push the function stack
                    stack.push_back(list<Token>());
                    
                    // Call the function
                    if(firstVar.var->getType() == FUNCTION)
                        firstVar.var = callFn(static_cast<Function*>(firstVar.var), tok2);
                    
                    // Pop the outer stack
                    vector<list<Token> >::iterator g = stack.end();
                    g--;
                    stack.erase(g);
                    
                    if(firstVar.type == Token::NOT_A_TOKEN)
                    {
                        return firstVar;
                    }
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis.\n");
                }
                else if(e->sep == OPEN_SQUARE_BRACKET)
                {
                    // FIXME: Array indexing
                    error("Syntax Error: Unexpected separator (FIXME: ARRAY INDEXING).\n");
                }
                else
                {
                    error("Syntax Error: Unexpected separator.\n");
                }
            }
            else if(e->type == Token::OPERATOR)
            {
                firstOp = *e;
                state = VAR_OP;
            }
            else
            {
                error("Syntax Error1: Unexpected token.  Expected an operator or parenthesis.\n");
            }
        }
        else if (state == VAR_OP)
        {
            if (e->type == Token::VARIABLE)
            {
                secondVar = *e;
                state = VAR_OP_VAR;
            }
            else if(e->type == Token::SEPARATOR)
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
                    secondVar = evalTokens(tok2, false);
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
                    secondVar = Token(getArrayLiteral(tokens, e), "");
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
                    error("Syntax Error: Unexpected token.  Expected a variable or parenthesis.\n");
            }
            else
                error("Syntax Error: Unexpected token.  Expected a variable or parenthesis.\n");
        }
        else if(state == VAR_OP_VAR)
        {
            if (e->type == Token::SEPARATOR)
            {
                if(e->sep == OPEN_PARENTHESIS)
                {
                    // VAR() -> Function call!
                    
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
                        secondVar.var = callFn(static_cast<Function*>(secondVar.var), tok2);
                    
                    // Pop the outer stack
                    vector<list<Token> >::iterator g = stack.end();
                    g--;
                    stack.erase(g);
                    
                    if(secondVar.type == Token::NOT_A_TOKEN)
                    {
                        return secondVar;
                    }
                }
                else if(e->sep == CLOSE_PARENTHESIS)
                {
                    error("Syntax Error: Unexpected closing parenthesis.\n");
                }
                else
                    error("Syntax Error: Unexpected separator.\n");
            }
            else if(e->type == Token::OPERATOR)
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
            else
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
                    else
                    {
                        error("Syntax Error3: Unexpected token.  Expected an operator or parenthesis.\n");
                    }
                }
                else if (state == VAR_OP)
                {
                    if (prev.type == Token::VARIABLE)
                    {
                        secondVar = Token(evaluateExpression(prev.var, firstOp.oper, secondVar.var), "<temp>");

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
                secondVar = Token(new Int(1), "<temp>");
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
