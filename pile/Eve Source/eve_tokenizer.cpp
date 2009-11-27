/*
eve_interpreter.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the implementation for the tokenizer, the part of the parsing
system that converts characters into meaningful tokens for use by the evaluater.
*/

#include "eve_interpreter.h"
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"

#include <string>
using namespace std;

extern Interpreter interpreter;


// Null token
Token::Token()
        : type(NOT_A_TOKEN)
        , var(NULL)
        , oper(NOT_AN_OPERATOR), precedence(0), associativeLeftToRight(true)
        , sep(NOT_A_SEPARATOR)
        , keyword(KW_NONE)
{}
// Variable token
Token::Token(Variable* var, std::string text)
        : type(VARIABLE)
        , text(text), var(var)
        , oper(NOT_AN_OPERATOR), precedence(0), associativeLeftToRight(true)
        , sep(NOT_A_SEPARATOR)
        , keyword(KW_NONE)
{
    if (var == NULL) // Error?
        type = NOT_A_TOKEN;
}
// Operator, Separator, or Keyword
Token::Token(TokenEnum type, std::string text)
        : type(type)
        , text(text)
        , var(NULL), oper(NOT_AN_OPERATOR), precedence(0), associativeLeftToRight(true)
{
    if(type == OPERATOR)
        setOperator(text);
    else if(type == SEPARATOR)
        setSeparator(text);
    else if(type == KEYWORD)
        setKeyword(text);
}

void Token::setKeyword(std::string Keyword)
{
    keyword = getKeyword(Keyword);
}

void Token::setOperator(std::string Oper)
{
    //strip(Oper, ' ');
    if (Oper == "+")
    {
        oper = ADD;
        precedence = 6;
    }
    if (Oper == "-")
    {
        oper = SUBTRACT;
        precedence = 6;
    }
    else if (Oper == "-X") // The weird one...
    {
        oper = NEGATE;
        associativeLeftToRight = false;
        precedence = 3;
    }
    else if (Oper == "=")
    {
        oper = ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "+=")
    {
        oper = ADD_ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "-=")
    {
        oper = SUBTRACT_ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "*=")
    {
        oper = MULTIPLY_ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "/=")
    {
        oper = DIVIDE_ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "%=")
    {
        oper = MODULUS_ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "**=")
    {
        oper = EXPONENTIATE_ASSIGN;
        associativeLeftToRight = false;
        precedence = 16;
    }
    else if (Oper == "*")
    {
        oper = MULTIPLY;
        precedence = 5;
    }
    else if (Oper == "/")
    {
        oper = DIVIDE;
        precedence = 5;
    }
    else if (Oper == "%")
    {
        oper = MODULUS;
        precedence = 5;
    }
    else if (Oper == "**")
    {
        oper = EXPONENTIATE;
        precedence = 2;
    }
    else if (Oper == "==")
    {
        oper = EQUALS;
        precedence = 9;
    }
    else if (Oper == "!=")
    {
        oper = NOT_EQUALS;
        precedence = 9;
    }
    else if (Oper == "<")
    {
        oper = LESS;
        precedence = 8;
    }
    else if (Oper == ">")
    {
        oper = GREATER;
        precedence = 8;
    }
    else if (Oper == "!<")
    {
        oper = NOT_LESS;
        precedence = 8;
    }
    else if (Oper == "!>")
    {
        oper = NOT_GREATER;
        precedence = 8;
    }
    else if (Oper == "<=")
    {
        oper = LESS_EQUAL;
        precedence = 8;
    }
    else if (Oper == ">=")
    {
        oper = GREATER_EQUAL;
        precedence = 8;
    }
    else if (Oper == "!")
    {
        oper = NOT;
        precedence = 3;
    }
    else if (Oper == "&&")
    {
        oper = AND;
        precedence = 13;
    }
    else if (Oper == "||")
    {
        oper = OR;
        precedence = 14;
    }
    /*else if (Oper == ",")
    {
        oper = COMMA;
        precedence = 18;
    }
    */
    else if (Oper == "...")
    {
        oper = CONTINUATION;
    }
    else if (Oper == ":")
    {
        oper = COLON;
        precedence = 1;
    }
    /*else if (Oper == ";")
    {
        oper = SEMICOLON;
    }*/
    else if (Oper == "&")
    {
        oper = BITWISE_AND;
        precedence = 10;
    }
    else if (Oper == "^")
    {
        oper = BITWISE_XOR;
        precedence = 11;
    }
    else if (Oper == "|")
    {
        oper = BITWISE_OR;
        precedence = 12;
    }
    else if (Oper == ".")
    {
        oper = DOT;
        precedence = 2;
    }
    else if (Oper == "[]")
    {
        oper = ARRAY_ACCESS;
        precedence = 2;
    }
    else if (Oper == "<>")
    {
        oper = HAS_ELEMENT;
        precedence = 8;
    }
    else if (Oper == "><")
    {
        oper = NOT_HAS_ELEMENT;
        precedence = 8;
    }
}

void Token::setSeparator(std::string s)
{
    //strip(s, ' ');
    if (s == ",")
    {
        sep = COMMA;
    }
    else if (s == "(")
    {
        sep = OPEN_PARENTHESIS;
    }
    else if (s == ")")
    {
        sep = CLOSE_PARENTHESIS;
    }
    else if (s == "[")
    {
        sep = OPEN_SQUARE_BRACKET;
    }
    else if (s == "]")
    {
        sep = CLOSE_SQUARE_BRACKET;
    }
    else if (s == "{")
    {
        sep = OPEN_CURLY_BRACKET;
    }
    else if (s == "}")
    {
        sep = CLOSE_CURLY_BRACKET;
    }
    else if (s == ";")
    {
        sep = SEMICOLON;
    }
}


std::string Token::getTypeString()
{
    switch (type)
    {
    case NOT_A_TOKEN:
        return "Not_A_Token";
    case VARIABLE:
        if (var->literal)
            return "Literal";
        if (var->getType() == TYPENAME)
            return "TypeName";
        return "Variable";
    case OPERATOR:
        return "Operator";
    case SEPARATOR:
        return "Separator";
    case KEYWORD:
        return "Keyword";
    default:
        return "Unknown Token type";
    }
}
std::string Token::getName()
{
    switch (type)
    {
    case NOT_A_TOKEN:
        return "";
    case VARIABLE:
        if (var->getType() == TYPENAME)
            return static_cast<TypeName*>(var)->getValueString();
        if (var->getType() == STRING)
            return static_cast<String*>(var)->getValueString();
        if (var->getType() == BOOL)
            return static_cast<Bool*>(var)->getValueString();
        if (var->getType() == INT)
            return static_cast<Int*>(var)->getValueString();
        if (var->getType() == FLOAT)
            return static_cast<Float*>(var)->getValueString();
        if (var->getType() == VOID)
            return static_cast<Void*>(var)->getValueString();
        return "Variable";
    case OPERATOR:
        return "Operator";
    case SEPARATOR:
        return "Separator";
    case KEYWORD:
        return "Keyword";
    default:
        return "Unknown Token type";
    }
}





TypeEnum getTypeFromString(const string& str)
{
    if(str == "void")
        return VOID;
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
    if(str == "class")
        return CLASS;
    if(str == "class_object")
        return CLASS_OBJECT;
    
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
        Bool* b = new Bool(true);
        b->literal = true;
        return b;
    }
    if(token == "false")
    {
        Bool* b = new Bool(false);
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
        TypeName* v = new TypeName(NOT_A_TYPE);
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




bool isOperator(const char& c)
{
    switch(c)
    {
        case '=':
            return true;
        case '+':
            return true;
        case '-':
            return true;
        case '*':
            return true;
        case '/':
            return true;
        case '<':
            return true;
        case '>':
            return true;
        case '!':
            return true;
        case '^':
            return true;
        case '&':
            return true;
        case '|':
            return true;
        case '%':
            return true;
        case '.':
            return true;
        case ':':
            return true;
    }
    return false;
}

bool isSeparator(const char& c)
{
    switch(c)
    {
        case '(':
            return true;
        case ')':
            return true;
        case ',':
            return true;
        case '[':
            return true;
        case ']':
            return true;
        case '{':
            return true;
        case '}':
            return true;
        case ';':
            return true;
    }
    return false;
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
            return (d == '=' || d == '*');
        case '/':
            return (d == '=');
        case '<':
            return (d == '=' || d == '<' || d == '>');
        case '>':
            return (d == '=' || d == '>' || d == '<');
        case '!':
            return (d == '=' || d == '>' || d == '<');
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

bool isTrigraphOperator(const char& c, const char& d, const char& e)
{
    return (c == '*' && d == '*' && e == '=');
}




KeywordEnum getKeyword(const string& str)
{
    if(str == "if")
        return KW_IF;
    if(str == "else")
        return KW_ELSE;
    if(str == "return")
        return KW_RETURN;
    return KW_NONE;
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
                String* s = new String("<temp>");
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
            String* s = new String("<temp>");
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
                if(i+1 < line.size() && isTrigraphOperator(line[i-1], line[i], line[i+1]))  // A trigraph operator...  "**="
                {
                    string token = line.substr(found, i - found + 2);
                    line.erase(0, i+2);
                    return Token(Token::OPERATOR, token);
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

