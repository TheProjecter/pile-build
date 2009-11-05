// FIXME: Remove Pile dependence
#include "eve_interpreter.h"
#include <fstream>
using namespace std;

extern Interpreter interpreter;


KeywordEnum getKeyword(const string& str)
{
    if(str == "if")
        return KW_IF;
    if(str == "else")
        return KW_ELSE;
    return KW_NONE;
}

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


Variable* equals(Variable* A, Variable* B)
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
            return new Bool(C->getValue() == D->getValue());
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
                return new Bool(C->getValue() == D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                return new Bool(C->getValue() == D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                return new Bool(C->getValue() == D->getValue());
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
                return new Bool(C->getValue() == D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                return new Bool(C->getValue() == D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                return new Bool(C->getValue() == D->getValue());
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
                return new Bool(C->getValue() == D->getValue());
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                return new Bool(C->getValue() == D->getValue());
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                return new Bool(C->getValue() == D->getValue());
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
            return new Bool(C->getValue() == D->getValue());
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
                return new Bool(C->getValue() == D->getValue());
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
            return new Bool(C->getValue() == D->getValue());
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
            return new Bool(C->getValue() == D->getValue());
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
            return new Bool(C->getValue() == D->getValue());
        }
    }
    
    //if(mismatch)
    {
        interpreter.error("Error: Types do not match in assignment: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return A;
    }
    return A;
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

Int* intCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Int(int(b->getValue()));
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Int(i->getValue());
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Int(int(f->getValue()));
    }
    return NULL;
}

Float* floatCast(Variable* v)
{
    if(v->getType() == BOOL)
    {
        Bool* b = static_cast<Bool*>(v);
        return new Float(float(b->getValue()));
    }
    if(v->getType() == INT)
    {
        Int* i = static_cast<Int*>(v);
        return new Float(float(i->getValue()));
    }
    if(v->getType() == FLOAT)
    {
        Float* f = static_cast<Float*>(v);
        return new Float(f->getValue());
    }
    return NULL;
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
            result = intCast(args[0]);
            break;
        case FN_FLOAT:
            if(args.size() != 1)
                return NULL;
            result = floatCast(args[0]);
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
