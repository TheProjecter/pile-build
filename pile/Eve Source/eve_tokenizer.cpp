#include "eve_interpreter.h"

#include <string>
using namespace std;

extern Interpreter interpreter;

TypeEnum getTypeFromString(const string& str)
{
    if(str == "string")
        return STRING;
    if(str == "int")
        return INT;
    if(str == "float")
        return FLOAT;
    if(str == "bool")
        return BOOL;
    if(str == "macro")
        return MACRO;
    if(str == "array")
        return ARRAY;
    if(str == "list")
        return LIST;
    if(str == "function")
        return FUNCTION;
    if(str == "procedure")
        return PROCEDURE;
    
    return NOT_A_TYPE;
}



// Analyze token to return a correct variable.
// First, see if it's a type name.
// If it's a literal (numbers only...  strings are handled in nextToken()),
// then return a new one.  
// Then, look it up in the environment.
// If it's not found, it's an error (maybe not?).
Variable* getCorrectVariable(string token, bool isFunction = false)
{
    if(token == "")
        return NULL;
    
    if(token == "true")
    {
        Bool* b = new Bool;
        b->setValue(true);
        b->literal = true;
        return b;
    }
    if(token == "false")
    {
        Bool* b = new Bool;
        b->setValue(false);
        b->literal = true;
        return b;
    }
    
    // #### or .####
    if(isNumeric(token[0]) || (token[0] == '.' && isNumeric(token[1])))
    {
        // Make a new numeric variable
        if(token.find_first_of('.') == string::npos)
        {
            // Int
            Int* i = new Int;
            int val = 0;
            sscanf(token.c_str(), "%d", &val);
            i->setValue(val);
            i->literal = true;
            return i;
        }
        else
        {
            // Float
            Float* f = new Float;
            float val = 0;
            sscanf(token.c_str(), "%f", &val);
            f->setValue(val);
            f->literal = true;
            return f;
        }
    }
    /*
    TODO: Construct other types here...
    */
    TypeEnum t = getTypeFromString(token);
    // Variable declaration
    if(!isFunction && t != NOT_A_TYPE)  // This happens if token states a type...
    {
        // Make a new type variable
        TypeName* v = new TypeName;
        v->setValue(t);
        return v;
    }
    
    Variable* var = interpreter.getVar(token);
    if(var == NULL)
    {
        var = new Void(token);
    }
    else
    {
        UI_debug_pile("Retrieved a variable: %s %s\n", var->getTypeString().c_str(), var->getValueString().c_str());
    }
    return var;
}




bool isDigraphOperator(const char& c, const char& d)
{
    switch(c)
    {
        case '=':
            return (d == '=');
        case '+':
            return (d == '=' || d == '+');
        case '-':
            return (d == '=' || d == '-');
        case '*':
            return (d == '=');
        case '/':
            return (d == '=');
        case '<':
            return (d == '=' || d == '<');
        case '>':
            return (d == '=' || d == '>');
        case '!':
            return (d == '=');
        case '^':
            return (d == '=');
        case '&':
            return (d == '=' || d == '&');
        case '|':
            return (d == '=' || d == '|');
        case '%':
            return (d == '=');
    }
    return false;
}






/*
Returns the next whitespace-delimited token and erases that token from the original string.
(destructive)

Takes: string (a raw line of text)
Returns: string (the next whitespace-delimited token)
*/
Token nextToken1(string& line, bool startingLine)
{
    // Ignore whitespace until you find something else.  Then find the whitespace on the other side
    // and return that string.
    int found = -1;
    bool foundOper = false;
    bool foundSep = false;
    bool foundAlpha = false;
    bool foundNumeric = false;
    bool foundQuote = false;
    bool dot = false;
    bool foundWhitespace = startingLine;
    for(unsigned int i = 0; i < line.size(); i++)
    {
        // A comment automatically ends the token.
        if(line[i] == '/' && line.size() > i && line[i+1] == '/')
        {
            if(found < 0)  // Nothing in this line except for a comment
            {
                line.clear();
                return Token();
            }
            string token = line.substr(found, i - found);
            line.clear();
            if(foundOper)  // Operator terminated by a comment
            {
                return Token(Token::OPERATOR, token);
            }
            if(foundSep)  // Separator terminated by a comment
            {
                return Token(Token::SEPARATOR, token);
            }
            if(foundAlpha || foundNumeric) // Variable terminated by a comment
            {
                return Token(getCorrectVariable(token), token);  // Will not be a function
            }
            if(foundQuote)  // FIXME: String terminated by a comment???  Maybe this should be an error? :)
            {
                String* s = new String;
                token = token.substr(1, string::npos);
                s->setValue(token);
                s->literal = true;
                return Token(s, token);
            }
            return Token();
        }
        
        if(foundQuote)  // If we're in a quoted string...
        {
            // FIXME: Should all of the escaped characters be done here?
            // FIXME: Allow single-quoted strings.
            if(line[i] == '\\')
            {
                if(line.size() <= i)
                {
                    // FIXME: Report error better...  Quoted line ended in a backslash...
                    return Token();
                }
                if(line[i+1] == '\"')
                {
                    // Skip the quote
                    line.erase(i, 1);
                    continue;
                }
                // Replace it with the real character
                line.erase(i, 1);
                if(line[i] == 'n')
                    line[i] = '\n';
                else if(line[i] == 't')
                    line[i] = '\t';
                else if(line[i] == '\'')
                    ;
                else if(line[i] == '?')
                    line[i] = '\?';
                else if(line[i] == '\\')
                    ;
                else if(line[i] == 'a')
                    line[i] = '\a';
                else if(line[i] == 'b')
                    line[i] = '\b';
                else if(line[i] == 'f')
                    line[i] = '\f';
                else if(line[i] == 'r')
                    line[i] = '\r';
                else if(line[i] == 'v')
                    line[i] = '\v';
                else
                {
                    // FIXME: Add better warning report for unknown escape sequence
                    return Token();
                }
                continue;
            }
            if(line[i] != '\"')  // Skip everything except the end of the quote.
                continue;
            // Return the quoted String
            string token = line.substr(found, i+1 - found);
            line.erase(0, i+1);
            token = token.substr(1, token.size()-2);
            String* s = new String;
            s->setValue(token);
            s->literal = true;
            return Token(s, token);
        }
        
        if(found < 0)  // If we haven't found anything yet...
        {
            if(isWhitespace(line[i]))  // Keep skipping whitespace
            {
                foundWhitespace = true;
                continue;
            }
            else  // We just found something
            {
                // Mark where it is and what it is.
                found = i;
                foundOper = isOperator(line[i]);
                foundSep = isSeparator(line[i]);
                foundAlpha = isAlpha(line[i]);
                foundNumeric = isNumeric(line[i]);
                foundQuote = (line[i] == '\"');
                dot = (line[i] == '.');
            }
            if(i+1 >= line.size())  // If the line is ending, return what we have.
            {
                string token = line.substr(found, i+1 - found);
                line.erase(0, i+1);
                if(foundOper)  // Operator
                {
                    return Token(Token::OPERATOR, token);
                }
                if(foundSep)  // Separator
                {
                    return Token(Token::SEPARATOR, token);
                }
                if(foundAlpha || foundNumeric) // Variable
                {
                    if(getKeyword(token) != KW_NONE)
                        return Token(Token::KEYWORD, token);
                    return Token(getCorrectVariable(token), token);  // Can't be a function
                }
                if(foundQuote)  // A lone quote?  Error?
                {
                    
                }
                if(dot)  // A lone dot?  Error?
                {
                    
                }
                return Token();
            }
        }
        else  // We found something earlier...
        {
            // Whitespace automatically ends the token
            if(isWhitespace(line[i]))  // Return the thing we found
            {
                string token = line.substr(found, i - found);
                line.erase(0, i);
                if(foundOper)  // Operator
                {
                    return Token(Token::OPERATOR, token);
                }
                if(foundSep)  // Separator
                {
                    return Token(Token::SEPARATOR, token);
                }
                if(foundAlpha || foundNumeric) // Variable
                {
                    if(getKeyword(token) != KW_NONE)
                        return Token(Token::KEYWORD, token);
                    return Token(getCorrectVariable(token), token);  // Can't be a function. "var ()" is no good?
                }
                if(foundQuote)  // A lone quote?  Error?
                {
                    
                }
                if(dot)  // A lone dot?  Error?
                {
                    
                }
                return Token();
            }
            if(foundOper)  // We've been tracking an operator...  " + " or " == " or "b==a", etc.
            {
                // Check the dot, because it's special
                if(line[found] == '.')  // We had found a dot at the beginning
                {
                    if(isAlpha(line[i]))  // Either a built-in function or an operator
                    {
                        if(foundWhitespace) // fn
                        {
                            // Start retrieving the whole token: " .thing"
                            foundOper = false;
                            dot = false;
                            foundAlpha = true;
                            UI_debug_pile("  Found a built-in function.\n");
                            continue;
                        }
                        else // operator
                        {
                            // Return the dot operator: "thing.thing"
                            string token = line.substr(found, i - found);  // Don't include this one
                            line.erase(0, i);  // Don't erase this one
                            return Token(Token::OPERATOR, token);
                        }
                    }
                    else if(isNumeric(line[i+1])) // a leading decimal point
                    {
                        foundOper = false;
                        dot = false;
                        foundNumeric = true;
                        continue;
                    }
                    else // Continuation operator "..." or an error...
                    {
                        if(line[i] == '.' && i+1 < line.size() && line[i+1] == '.')
                        {
                            string token = line.substr(found, i - found + 2);
                            line.erase(0, i+2);
                            return Token(Token::OPERATOR, token);
                        }
                        
                        string token = line.substr(found, i - found + 1);
                        line.erase(0, i+1);
                        return Token();
                    }
                }
                if(isDigraphOperator(line[i-1], line[i]))  // A digraph operator...  "==", ">=", "+=", etc.
                {
                    string token = line.substr(found, i - found + 1);
                    line.erase(0, i+1);
                    return Token(Token::OPERATOR, token);
                }
                // Hit something else
                string token = line.substr(found, i - found); // Don't include this one
                line.erase(0, i);  // Don't erase this one
                return Token(Token::OPERATOR, token);
            }
            if(foundSep)  // We've been tracking a separator...
            {
                // Hit something else
                string token = line.substr(found, i - found); // Don't include this one
                line.erase(0, i);  // Don't erase this one
                return Token(Token::SEPARATOR, token);
            }
            if(foundNumeric)
            {
                if(line[i] == '.')
                {
                    if(dot)
                    {
                        // Error, too many dots
                    }
                    else
                        dot = true;
                }
                else if(!isNumeric(line[i])) // End of digits
                {
                    // Return the token
                    string token = line.substr(found, i - found); // Don't include this one
                    line.erase(0, i);  // Don't erase this one
                    return Token(getCorrectVariable(token), token);  // Is a number, not a function
                }
            }
            if(foundAlpha)  // Found a variable
            {
                if(!isAlphanumeric(line[i]))  // Hit something while tracking a variable
                {
                    bool isFn = (line[i] == '(');
                    string token = line.substr(found, i - found); // Don't include this one
                    line.erase(0, i);  // Don't erase this one
                    if(getKeyword(token) != KW_NONE)
                        return Token(Token::KEYWORD, token);
                    return Token(getCorrectVariable(token, isFn), token);
                }
            }
            if(i+1 >= line.size())  // The line is ending, return what we have.
            {
                string token = line.substr(found, i+1 - found);
                line.erase(0, i+1);
                if(foundOper)  // Operator
                {
                    return Token(Token::OPERATOR, token);
                }
                if(foundSep)  // Separator
                {
                    return Token(Token::SEPARATOR, token);
                }
                if(foundAlpha || foundNumeric) // Variable
                {
                    if(getKeyword(token) != KW_NONE)
                        return Token(Token::KEYWORD, token);
                    return Token(getCorrectVariable(token), token);  // Can't be a function
                }
                if(foundQuote)  // A lone quote?  Error?
                {
                    
                }
            }
        }
    }
    return Token();
}


/*
Breaks a raw line of text into whitespace-delimited tokens.

Takes: string (line of text)
Returns: list<Token> (separated tokens)
*/
list<Token> tokenize1(string& line, bool& continuation)
{
    list<Token> tokens;
    Token tok;
    #ifdef PILE_DEBUG_TOKENS
        static int tokenNum = 1;
    #endif
    
    continuation = false;
    
    bool start = true;
    do
    {
        tok = nextToken1(line, start);
        start = false;
        #ifdef PILE_DEBUG_TOKENS
            UI_debug_pile("Token %d: <%s> \"%s\" : \"%s\"\n", tokenNum, tok.getTypeString().c_str(), tok.getName().c_str(), tok.text.c_str());
            tokenNum++;
        #endif
        if(tok.type != Token::NOT_A_TOKEN)
        {
            tokens.push_back(tok);
        }
    }
    while(line.size() > 0 && tok.type != Token::NOT_A_TOKEN);
    
    // Check for the continuation operator at the end.
    if(tokens.size() > 0)
    {
        list<Token>::iterator e = tokens.end();
        e--;
        if(e->type == Token::OPERATOR && e->oper == CONTINUATION)
        {
            continuation = true;
            tokens.erase(e);
        }
    }
    
    return tokens;
}

