// FIXME: Remove Pile dependence
#include "eve_interpreter.h"
#include <fstream>
using namespace std;

extern Interpreter interpreter;


bool isConvertable(TypeEnum source, TypeEnum dest)
{
    if(source == INT)
    {
        return (dest == BOOL || dest == INT || dest == FLOAT);
    }
    if(source == FLOAT)
    {
        return (dest == BOOL || dest == INT || dest == FLOAT);
    }
    if(source == STRING)
    {
        return (dest == STRING);
    }
    // FIXME: Finish this.
    return false;
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
            evalTokens(tokens, true);
            lineNumber++;
        }
        else if(continuation)
            lineNumber++;  // Skip an extra line if the last one was continued...  This probably isn't right for multiple-line continues...
    }
    
    fin.close();
    currentFile = "";
    
    return !errorFlag;
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

string getOperatorString(OperatorEnum type)
{
    switch(type)
    {
        case ADD:
            return "+";
        case SUBTRACT:
            return "-";
        case NEGATE:
            return "-";
        case ASSIGN:
            return "=";
        case ADD_ASSIGN:
            return "+=";
        case SUBTRACT_ASSIGN:
            return "-=";
        case MULTIPLY_ASSIGN:
            return "*=";
        case DIVIDE_ASSIGN:
            return "/=";
        case MULTIPLY:
            return "*";
        case DIVIDE:
            return "/";
        case MODULUS:
            return "%";
        case EQUALS:
            return "==";
        case NOT_EQUALS:
            return "!=";
        case LESS:
            return "<";
        case GREATER:
            return ">";
        case LESS_EQUAL:
            return "<=";
        case GREATER_EQUAL:
            return ">=";
        case NOT:
            return "!";
        case AND:
            return "&&";
        case OR:
            return "||";
        /*case COMMA:
            return ",";*/
        case CALL:
            return "()";
        /*case OPEN_PARENTHESIS:
            return "(";
        case CLOSE_PARENTHESIS:
            return ")";*/
        case CONTINUATION:
            return "...";
        /*case OPEN_SQUARE_BRACKET:
            return "[";
        case CLOSE_SQUARE_BRACKET:
            return "]";
        case OPEN_CURLY_BRACKET:
            return "[";
        case CLOSE_CURLY_BRACKET:
            return "]";*/
        case COLON:
            return ":";
        /*case SEMICOLON:
            return ";";*/
        default:
            return "N/A";
    }
}

string getSeparatorString(SeparatorEnum type)
{
    switch(type)
    {
        case COMMA:
            return ",";
        case OPEN_PARENTHESIS:
            return "(";
        case CLOSE_PARENTHESIS:
            return ")";
        case OPEN_SQUARE_BRACKET:
            return "[";
        case CLOSE_SQUARE_BRACKET:
            return "]";
        case OPEN_CURLY_BRACKET:
            return "[";
        case CLOSE_CURLY_BRACKET:
            return "]";
        case SEMICOLON:
            return ";";
        default:
            return "N/A";
    }
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



string getTypeString(TypeEnum type)
{
    switch(type)
    {
        case BOOL:
            return "bool";
        case INT:
            return "int";
        case FLOAT:
            return "float";
        case STRING:
            return "string";
        case MACRO:
            return "macro";
        case ARRAY:
            return "array";
        case LIST:
            return "list";
        case FUNCTION:
            return "function";
        case PROCEDURE:
            return "procedure";
        default:
            return "unknown";
    }
}



Variable* assign(Variable* A, Variable* B)
{
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    
    bool mismatch = false;
    if(a == STRING)
    {
        if(b != STRING)
            mismatch = true;
        else
        {
            String* C = static_cast<String*>(A);
            String* D = static_cast<String*>(B);
            C->setValue(D->getValue());
        }
    }
    else if(a == INT)
    {
        if(b != BOOL && b != INT && b != FLOAT)
            mismatch = true;
        else
        {
            Int* C = static_cast<Int*>(A);
            if(b == BOOL)
            {
                Bool* D = static_cast<Bool*>(B);
                C->setValue(D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                C->setValue(D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                C->setValue(D->getValue());
            }
        }
    }
    else if(a == FLOAT)
    {
        if(b != BOOL && b != INT && b != FLOAT)
            mismatch = true;
        else
        {
            Float* C = static_cast<Float*>(A);
            if(b == BOOL)
            {
                Bool* D = static_cast<Bool*>(B);
                C->setValue(D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                C->setValue(D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                C->setValue(D->getValue());
            }
        }
    }
    else if(a == BOOL)
    {
        if(b != BOOL && b != INT && b != FLOAT)
            mismatch = true;
        else
        {
            Bool* C = static_cast<Bool*>(A);
            if(b == BOOL)
            {
                Bool* D = static_cast<Bool*>(B);
                C->setValue(D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                C->setValue(D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                C->setValue(D->getValue());
            }
        }
    }
    else if(a == MACRO)
    {
        if(b != MACRO)
            mismatch = true;
        else
        {
            Macro* C = static_cast<Macro*>(A);
            Macro* D = static_cast<Macro*>(B);
            C->setValue(D->getValue());
        }
    }
    else if(a == ARRAY)
    {
        if(b != ARRAY)
            mismatch = true;
        else
        {
            Array* C = static_cast<Array*>(A);
            Array* D = static_cast<Array*>(B);
            a = C->getValueType();
            b = C->getValueType();
            if(a != b)
            {
                interpreter.error("Error: Types do not match in assignment: Array<%s> vs Array<%s>\n", C->getValueTypeString().c_str(), D->getValueTypeString().c_str());
                return A;
            }
            C->setValue(D->getValue());
        }
    }
    else if(a == LIST)
    {
        if(b != LIST)
            mismatch = true;
        else
        {
            List* C = static_cast<List*>(A);
            List* D = static_cast<List*>(B);
            C->setValue(D->getValue());
        }
    }
    else if(a == FUNCTION)
    {
        if(b != FUNCTION)
            mismatch = true;
        else
        {
            Function* C = static_cast<Function*>(A);
            Function* D = static_cast<Function*>(B);
            C->setValue(D->getValue());
        }
    }
    else if(a == PROCEDURE)
    {
        if(b != PROCEDURE)
            mismatch = true;
        else
        {
            Procedure* C = static_cast<Procedure*>(A);
            Procedure* D = static_cast<Procedure*>(B);
            C->setValue(D->getValue());
        }
    }
    
    if(mismatch)
    {
        interpreter.error("Error: Types do not match in assignment: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return A;
    }
    return A;
}


Variable* add_assign(Variable* A, Variable* B)
{
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    
    bool mismatch = false;
    if(a == STRING)
    {
        if(b != STRING)
            mismatch = true;
        else
        {
            String* C = static_cast<String*>(A);
            String* D = static_cast<String*>(B);
            C->setValue(C->getValue() + D->getValue());
        }
    }
    else if(a == INT)
    {
        if(b != BOOL && b != INT && b != FLOAT)
            mismatch = true;
        else
        {
            Int* C = static_cast<Int*>(A);
            if(b == BOOL)
            {
                Bool* D = static_cast<Bool*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
        }
    }
    else if(a == FLOAT)
    {
        if(b != BOOL && b != INT && b != FLOAT)
            mismatch = true;
        else
        {
            Float* C = static_cast<Float*>(A);
            if(b == BOOL)
            {
                Bool* D = static_cast<Bool*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
        }
    }
    else if(a == BOOL)
    {
        if(b != BOOL && b != INT && b != FLOAT)
            mismatch = true;
        else
        {
            Bool* C = static_cast<Bool*>(A);
            if(b == BOOL)
            {
                Bool* D = static_cast<Bool*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                C->setValue(C->getValue() + D->getValue());
            }
        }
    }
    else if(a == MACRO)
    {
        interpreter.error("Error: Addition operation not defined for type 'macro'.\n");
        return A;
    }
    else if(a == ARRAY)
    {
        if(b == ARRAY)
        {
            Array* C = static_cast<Array*>(A);
            Array* D = static_cast<Array*>(B);
            a = C->getValueType();
            b = C->getValueType();
            if(a != b)
            {
                interpreter.error("Error: Types do not match in assignment: Array<%s> vs Array<%s>\n", C->getValueTypeString().c_str(), D->getValueTypeString().c_str());
                return A;
            }
            C->push_back(D->getValue());
        }
        else
        {
            Array* C = static_cast<Array*>(A);
            if(b == C->getValueType())
            {
                C->push_back(B);
            }
            else
                mismatch = true;
        }
    }
    else if(a == LIST)
    {
        // Lists must be concatenated a different way...
        List* C = static_cast<List*>(A);
        C->push_back(B);
    }
    else if(a == FUNCTION)
    {
        interpreter.error("Error: Addition operation not defined for type 'function'.\n");
        return A;
    }
    else if(a == PROCEDURE)
    {
        interpreter.error("Error: Addition operation not defined for type 'procedure'.\n");
        return A;
    }
    
    if(mismatch)
    {
        interpreter.error("Error: Types do not match in assignment: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return A;
    }
    return A;
}

Variable* subtract_assign(Variable* A, Variable* B)
{
    Variable* C = subtract(A, B);
    C = assign(A, C);
    return C;
}

Variable* multiply_assign(Variable* A, Variable* B)
{
    Variable* C = multiply(A, B);
    C = assign(A, C);
    return C;
}

Variable* divide_assign(Variable* A, Variable* B)
{
    Variable* C = divide(A, B);
    C = assign(A, C);
    return C;
}

Variable* add(Variable* A, Variable* B)
{
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a != b && !(a == FLOAT && b == INT) && !(b == FLOAT && a == INT))
    {
        interpreter.error("Error: Types do not match in addition: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return A;
    }
    if(a == STRING)
    {
        String* C = static_cast<String*>(A);
        String* D = static_cast<String*>(B);
        String* R = new String;
        R->setValue(C->getValue() + D->getValue());
        return R;
    }
    else if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Int* R = new Int;
            R->setValue(C->getValue() + D->getValue());
            return R;
        }
        else
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() + D->getValue());
            return R;
        }
    }
    else if(a == FLOAT)
    {
        Float* C = static_cast<Float*>(A);
        Float* R = new Float;
        
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            R->setValue(C->getValue() + D->getValue());
        }
        else
        {
            Float* D = static_cast<Float*>(B);
            R->setValue(C->getValue() + D->getValue());
        }
        return R;
    }
    else if(a == BOOL)
    {
        interpreter.error("Error: Addition operation not defined for type 'bool'.\n");
        return A;
    }
    else if(a == MACRO)
    {
        Macro* C = static_cast<Macro*>(A);
        Macro* D = static_cast<Macro*>(B);
        Macro* R = new Macro;
        R->setValue(C->getValue() + D->getValue());
        return R;
    }
    else if(a == ARRAY)
    {
        Array* C = static_cast<Array*>(A);
        Array* D = static_cast<Array*>(B);
        a = C->getValueType();
        b = D->getValueType();
        if(a != b)
        {
            interpreter.error("Error: Types do not match in addition: Array<%s> vs Array<%s>\n", C->getValueTypeString().c_str(), D->getValueTypeString().c_str());
            return A;
        }
        
        vector<Variable*> va = C->getValue();
        vector<Variable*>& vb = D->getValue();
        
        for(vector<Variable*>::iterator e = vb.begin(); e != vb.end(); e++)
        {
            va.push_back(*e);
        }
        
        Array* arr = new Array(va, a);
        
        return arr;
    }
    else if(a == LIST)
    {
        List* C = static_cast<List*>(A);
        List* D = static_cast<List*>(B);
        list<Variable*> va = C->getValue();
        list<Variable*>& vb = D->getValue();
        
        for(list<Variable*>::iterator e = vb.begin(); e != vb.end(); e++)
        {
            va.push_back(*e);
        }
        
        List* lst = new List(va);
        
        return lst;
    }
    else if(a == FUNCTION)
    {
        Function* C = static_cast<Function*>(A);
        Function* D = static_cast<Function*>(B);
        Function* R = new Function;
        R->setValue(C->getValue() + D->getValue());
        return R;
    }
    else if(a == PROCEDURE)
    {
        Procedure* C = static_cast<Procedure*>(A);
        Procedure* D = static_cast<Procedure*>(B);
        Procedure* R = new Procedure;
        R->setValue(C->getValue() + D->getValue());
        return R;
    }
    return A;
}

Variable* subtract(Variable* A, Variable* B)
{
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a != b && !(a == FLOAT && b == INT) && !(b == FLOAT && a == INT))
    {
        interpreter.error("Error: Types do not match in subtraction: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return A;
    }
    if(a == STRING)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'string'.\n");
        return A;
    }
    else if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Int* R = new Int;
            R->setValue(C->getValue() - D->getValue());
            return R;
        }
        else
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() - D->getValue());
            return R;
        }
    }
    else if(a == FLOAT)
    {
        Float* C = static_cast<Float*>(A);
        Float* R = new Float;
        
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            R->setValue(C->getValue() - D->getValue());
        }
        else
        {
            Float* D = static_cast<Float*>(B);
            R->setValue(C->getValue() - D->getValue());
        }
        return R;
    }
    else if(a == BOOL)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'bool'.\n");
        return A;
    }
    else if(a == MACRO)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'macro'.\n");
        return A;
    }
    else if(a == ARRAY)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'array'.\n");
        return A;
    }
    else if(a == LIST)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'list'.\n");
        return A;
    }
    else if(a == FUNCTION)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'function'.\n");
        return A;
    }
    else if(a == PROCEDURE)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'procedure'.\n");
        return A;
    }
    return A;
}

Variable* multiply(Variable* A, Variable* B)
{
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE
      || b == STRING || b == BOOL || b == MACRO || b == ARRAY || b == LIST || b == FUNCTION || b == PROCEDURE)
    {
        interpreter.error("Error: Multiplication not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return A;
    }
    if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Int* R = new Int;
            R->setValue(C->getValue() * D->getValue());
            return R;
        }
        else if(b == FLOAT)
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() * D->getValue());
            return R;
        }
    }
    else if(a == FLOAT)
    {
        Float* C = static_cast<Float*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() * D->getValue());
            return R;
        }
        else if(b == FLOAT)
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() * D->getValue());
            return R;
        }
    }
    return A;
}

Variable* divide(Variable* A, Variable* B)
{
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE
      || b == STRING || b == BOOL || b == MACRO || b == ARRAY || b == LIST || b == FUNCTION || b == PROCEDURE)
    {
        interpreter.error("Error: Division not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return A;
    }
    if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Int* R = new Int;
            R->setValue(C->getValue() / D->getValue());
            return R;
        }
        else if(b == FLOAT)
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() / D->getValue());
            return R;
        }
    }
    else if(a == FLOAT)
    {
        Float* C = static_cast<Float*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() / D->getValue());
            return R;
        }
        else if(b == FLOAT)
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(C->getValue() / D->getValue());
            return R;
        }
    }
    return A;
}

Variable* negate(Variable* A)
{
    TypeEnum a = A->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE)
    {
        interpreter.error("Error: Negation not defined for type '%s'\n", getTypeString(a).c_str());
        return A;
    }
    if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        Int* R = new Int;
        R->setValue(-C->getValue());
        return R;
    }
    else if(a == FLOAT)
    {
        Float* C = static_cast<Float*>(A);
        Float* R = new Float;
        R->setValue(-C->getValue());
        return R;
    }
    return A;
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
        
        Token t = evalTokens(tok2, false);
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
            else
            {
                error("Syntax Error: Unexpected token at the beginning of a line.\n");
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
        case FN_INT:
            if(args.size() != 1)
                return NULL;
            result = NULL;
            // FIXME!
            //result = new Int(args[0]->getValue());
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
