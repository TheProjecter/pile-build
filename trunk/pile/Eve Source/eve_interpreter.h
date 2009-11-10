/*
Eve, an interpreted programming language.
by Jonathan Dearborn
*/


#ifndef _EVE_INTERPRETER_H__
#define _EVE_INTERPRETER_H__

#include <sstream>
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"
#include <list>
#include <vector>
#include <map>
#include <string>
#include <istream>
#include <sstream>
#include "stdarg.h"  // FIXME: Move into the cpp file

bool isWhitespace(const char& c);

bool isMatch(const char& c, const std::vector<char>& matching);

bool isQuantizer(const char& c);

inline bool isAlpha(const char& c)
{
    return (65 <= c && c <= 90) || (97 <= c && c <= 122) || c == '_';
}

inline bool isNumeric(const char& c)
{
    return (48 <= c && c <= 57);
}

inline bool isAlphanumeric(const char& c)
{
    return isAlpha(c) || isNumeric(c);
}


bool isOperator(const char& c);
bool isSeparator(const char& c);


enum TypeEnum{NOT_A_TYPE, VOID, TYPENAME, BOOL, INT, FLOAT, STRING, MACRO, ARRAY, LIST, FUNCTION, PROCEDURE, CLASS, CLASS_OBJECT};

enum OperatorEnum{NOT_AN_OPERATOR, ADD, SUBTRACT, NEGATE, ASSIGN, ADD_ASSIGN, SUBTRACT_ASSIGN, MULTIPLY_ASSIGN, DIVIDE_ASSIGN,
                  MULTIPLY, DIVIDE, MODULUS, EQUALS, NOT_EQUALS, LESS, GREATER, NOT_LESS, NOT_GREATER, LESS_EQUAL,
                  GREATER_EQUAL, NOT, AND, OR, CALL, CONTINUATION, COLON, DOT, BITWISE_AND, BITWISE_XOR, BITWISE_OR
                 };
                 
enum SeparatorEnum{NOT_A_SEPARATOR, COMMA, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, 
                OPEN_SQUARE_BRACKET, CLOSE_SQUARE_BRACKET, OPEN_CURLY_BRACKET, 
                CLOSE_CURLY_BRACKET, SEMICOLON
                 };

enum KeywordEnum{KW_NONE, KW_IF, KW_ELSE, KW_RETURN};

enum FunctionEnum{FN_NONE, FN_PRINT, FN_PRINTLN, FN_TYPE, FN_STRING, FN_BOOL, FN_INT, FN_FLOAT};

KeywordEnum getKeyword(const std::string& str);

class Token;
std::list<Token> tokenize1(std::string& line, bool& continuation);

std::string getTypeString(TypeEnum type);
std::string getOperatorString(OperatorEnum type);
std::string getSeparatorString(SeparatorEnum type);

class Interpreter;
class Variable;
Variable* callBuiltIn(FunctionEnum fn, std::vector<Variable*>& args);


bool isConvertable(TypeEnum source, TypeEnum dest);

class Variable
{
protected:
    TypeEnum type;
public:
    bool literal;
    bool temp;
    bool reference;  // Used as a message to callFn that this should be passed by reference.
    Variable(TypeEnum type)
            : type(type)
            , literal(false)
            , temp(false)
            , reference(false)
    {}
    TypeEnum getType()
    {
        return type;
    }
    virtual Variable* copy() = 0;
    virtual std::string getValueString() = 0;
    // This could easily be a virtual function instead... and then it'd be independent of some external stuff.
    std::string getTypeString()
    {
        return ::getTypeString(type);
    }
    Variable* makeTemp()
    {
        temp = true;
        return this;
    }
};

// This class represents a type declaration.
class TypeName : public Variable
{
private:
    TypeEnum value; // What type is being declared?
public:
    std::string text;
    TypeName()
            : Variable(TYPENAME)
    {}
    TypeName(TypeEnum type)
            : Variable(TYPENAME)
            , value(type)
            , text(::getTypeString(type))
    {}
    TypeName(TypeEnum type, const std::string& text)
            : Variable(TYPENAME)
            , value(type)
            , text(text)
    {}
    void setValue(const TypeEnum& val)
    {
        value = val;
    }
    TypeEnum& getValue()
    {
        return value;
    }
    virtual std::string getValueString()
    {
        return ::getTypeString(value);
    }
    virtual Variable* copy()
    {
        Variable* cp = new TypeName(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

// This class represents an undefined variable.
class Void : public Variable
{
private:
    std::string value; // What type is being declared?
public:
    Void()
            : Variable(VOID)
    {}
    Void(const std::string& val)
            : Variable(VOID)
            , value(val)
    {}
    void setValue(const std::string& val)
    {
        value = val;
    }
    std::string& getValue()
    {
        return value;
    }
    virtual std::string getValueString()
    {
        return value;
    }
    virtual Variable* copy()
    {
        Variable* cp = new Void(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class String : public Variable
{
private:
    std::string value;
public:
    String()
            : Variable(STRING)
    {}
    String(const std::string& value)
            : Variable(STRING)
            , value(value)
    {}
    void setValue(const std::string& val)
    {
        value = val;
    }
    std::string& getValue()
    {
        return value;
    }
    virtual std::string getValueString()
    {
        return value;
    }
    virtual Variable* copy()
    {
        Variable* cp = new String(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class Int : public Variable
{
private:
    int value;
public:
    Int()
            : Variable(INT)
            , value(0)
    {}
    Int(int value)
            : Variable(INT)
            , value(value)
    {}
    int& getValue()
    {
        return value;
    }
    void setValue(const int& val)
    {
        value = val;
    }
    virtual std::string getValueString()
    {
        char buff[20];
        sprintf(buff, "%d", value);
        return buff;
    }
    virtual Variable* copy()
    {
        Variable* cp = new Int(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class Float : public Variable
{
private:
    float value;
public:
    Float()
            : Variable(FLOAT)
            , value(0.0f)
    {}
    Float(float value)
            : Variable(FLOAT)
            , value(value)
    {}
    float& getValue()
    {
        return value;
    }
    void setValue(const float& val)
    {
        value = val;
    }
    virtual std::string getValueString()
    {
        char buff[200];
        sprintf(buff, "%f", value);
        return buff;
    }
    virtual Variable* copy()
    {
        Variable* cp = new Float(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class Bool : public Variable
{
private:
    bool value;
public:
    Bool()
            : Variable(BOOL)
            , value(false)
    {}
    Bool(bool value)
            : Variable(BOOL)
            , value(value)
    {}
    bool& getValue()
    {
        return value;
    }
    void setValue(const bool& val)
    {
        value = val;
    }
    virtual std::string getValueString()
    {
        return (value? "true" : "false");
    }
    virtual Variable* copy()
    {
        Variable* cp = new Bool(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class Macro : public Variable
{
private:
    std::string value;
public:
    Macro()
            : Variable(MACRO)
    {}
    std::string& getValue()
    {
        return value;
    }
    void setValue(const std::string& val)
    {
        value = val;
    }
    virtual std::string getValueString()
    {
        return value;
    }
    virtual Variable* copy()
    {
        Variable* cp = new Macro(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};



class Array : public Variable
{
private:
    TypeEnum valueType;
    std::vector<Variable*> value;
public:
    Array()
            : Variable(ARRAY)
            , valueType(VOID)
    {}
    Array(TypeEnum valueType)
            : Variable(ARRAY)
            , valueType(valueType)
    {}
    Array(const std::vector<Variable*>& value, TypeEnum valueType)
            : Variable(ARRAY)
            , valueType(valueType)
            , value(value)
    {}
    std::vector<Variable*>& getValue()
    {
        return value;
    }
    void setValue(const std::vector<Variable*>& val)
    {
        value = val;  // Needs copy
    }
    std::string getValueTypeString()
    {
        return ::getTypeString(valueType);
    }
    TypeEnum getValueType()
    {
        return valueType;
    }
    virtual std::string getValueString()
    {
        std::string result;
        for (std::vector<Variable*>::iterator e = value.begin(); e != value.end();)
        {
            result += (*e)->getValueString();
            e++;
            if(e != value.end())
                result += ", ";
        }
        return result;
    }
    void push_back(Variable* var)
    {
        if(var != NULL && var->getType() == valueType)
            value.push_back(var);
    }
    void push_back(const std::vector<Variable*>& vec)
    {
        for(std::vector<Variable*>::const_iterator e = vec.begin(); e != vec.end(); e++)
        {
            if((*e)->getType() == valueType)
                value.push_back(*e);
        }
    }
    virtual Variable* copy()
    {
        Variable* cp = new Array(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class List : public Variable
{
private:
    std::list<Variable*> value;
public:
    List()
            : Variable(LIST)
    {}
    List(const std::list<Variable*>& value)
            : Variable(LIST)
            , value(value)
    {}
    std::list<Variable*>& getValue()
    {
        return value;
    }
    void setValue(const std::list<Variable*>& val)
    {
        value = val;  // Needs copy
    }
    virtual std::string getValueString()
    {
        char buff[2048];
        buff[0] = '<';
        char* c = buff+1;
        int num = 1;
        bool ran = false;
        for (std::list<Variable*>::iterator e = value.begin(); e != value.end(); e++)
        {
            if (ran)
            {
                sprintf(c, ", ");
                c += 1;
            }
            ran = true;
            sprintf(c, "(%s, '%s')%n", (*e)->getTypeString().c_str(), (*e)->getValueString().c_str(), &num);
            c += num;  // Move the pointer along...
        }
        buff[num] = '>';
        buff[num+1] = '\0';
        return buff;
    }
    void push_back(Variable* var)
    {
        if(var != NULL)
            value.push_back(var);
    }
    virtual Variable* copy()
    {
        Variable* cp = new List(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class Function : public Variable
{
private:
    std::string value;
    std::vector<TypeName> argt;  // Types of arguments
    std::vector<std::string> args;  // Names of arguments
    FunctionEnum builtIn;
public:
    TypeName returnType;

    std::string definitionFile;
    unsigned int lineNumber;


    Function()
            : Variable(FUNCTION)
            , builtIn(FN_NONE)
            , lineNumber(0)
    {}
    Function(const std::vector<TypeName>& argt, const std::string& value)
            : Variable(FUNCTION)
            , value(value)
            , argt(argt)
            , builtIn(FN_NONE)
            , lineNumber(0)
    {}
    Function(FunctionEnum builtIn)
            : Variable(FUNCTION)
            , builtIn(builtIn)
            , lineNumber(0)
    {
        switch(builtIn)
        {
            case FN_PRINT:
                argt.push_back(TypeName(STRING));
                break;
            case FN_PRINTLN:
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
            case FN_NONE:
                break;
        }
    }
    std::string& getValue()
    {
        return value;
    }
    std::vector<TypeName>& getArgTypes()
    {
        return argt;
    }
    void setValue(const std::string& val)
    {
        value = val;
    }
    void setArgTypes(const std::vector<TypeName>& argTypes)
    {
        argt = argTypes;
    }
    void addArg(const TypeName& argType, const std::string& argName)
    {
        argt.push_back(argType);
        args.push_back(argName);
    }
    /*void loadFromSig(const std::list<Token>& fnSignature)
    {
        // ...
    }*/
    virtual std::string getValueString()
    {
        return value;
    }
    bool isBuiltIn()
    {
        return (builtIn != FN_NONE);
    }
    FunctionEnum getBuiltIn()
    {
        return builtIn;
    }
    Variable* call(Interpreter& interpreter, std::vector<Variable*>& args);
    
    virtual Variable* copy()
    {
        Variable* cp = new Function(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class Procedure : public Variable
{
private:
    std::string value;
public:
    Procedure()
            : Variable(PROCEDURE)
    {}
    std::string& getValue()
    {
        return value;
    }
    void setValue(const std::string& val)
    {
        value = val;
    }
    virtual std::string getValueString()
    {
        return value;
    }
    virtual Variable* copy()
    {
        Variable* cp = new Procedure(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};



class Class : public Variable
{
private:
    std::string name;
public:
    class VarRecord
    {
        public:
        std::string type;
        std::string name;
        std::list<std::string> functionArgs;
        std::string functionDefinition;
        VarRecord(const std::string& type, const std::string& name)
            : type(type), name(name)
        {}
        VarRecord(const std::string& name, const std::list<std::string> argtypes, const std::string& definition)
            : type("function"), name(name), functionArgs(argtypes), functionDefinition(definition)
        {}
    };
    std::list<VarRecord> vars;
    
    Class()
            : Variable(CLASS)
    {}
    Class(const std::string& name)
            : Variable(CLASS)
            , name(name)
    {}
    void addVariable(const std::string& vartype, const std::string& varname)
    {
        vars.push_back(VarRecord(vartype, varname));
    }
    void addFunction(const std::string& varname, const std::list<std::string> argtypes, const std::string& definition)
    {
        vars.push_back(VarRecord(varname, argtypes, definition));
    }
    virtual std::string getValueString()
    {
        return name;
    }
    virtual Variable* copy()
    {
        Variable* cp = new Class(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};

class ClassObject : public Variable
{
private:
    std::string name;
    std::map<std::string, Variable*> vars;
    
    void addVariables()
    {
        // Use classname and check the interpreter's list of class definitions
        // to create new variables and add them to the map.
        //vars.insert(std::make_pair(varname, var));
    }
    
public:
    std::string className;
    ClassObject()
            : Variable(CLASS)
    {}
    ClassObject(const std::string& name)
            : Variable(CLASS)
            , name(name)
    {
        // FIXME
        // Search the registered classes for the name
        // If it's found, create all new variables.
        // If it's not found, set className to ""
    }
    Variable* getVariable(const std::string& var)
    {
        std::map<std::string, Variable*>::iterator e = vars.find(var);
        if(e == vars.end())
            return NULL;
        return e->second;
    }
    virtual std::string getValueString()
    {
        return name;
    }
    virtual Variable* copy()
    {
        Variable* cp = new ClassObject(*this);
        cp->temp = false;
        cp->reference = false;
        return cp;
    }
};


Variable* comparison(Variable* A, Variable* B, OperatorEnum oper);

Variable* assign(Variable* A, Variable* B);

Variable* add(Variable* A, Variable* B);

Variable* subtract(Variable* A, Variable* B);

Variable* negate(Variable* A);

Variable* multiply(Variable* A, Variable* B);

Variable* divide(Variable* A, Variable* B);

Variable* add_assign(Variable* A, Variable* B);

class Scope
{
public:
    bool isolated;  // If true, don't check previous scopes for variables (except for global).
    // This is used for functions, which have their own separate scope.
    
    
    enum ScopeEnum {NO_BLOCK, IF_BLOCK, SKIP_IF, SKIP_ELSE};
    // SKIP_IF means that the if() evaluated to false.  We skip the if block and
    // eval the else blocks.
    // SKIP_ELSE means that the if() evaluated to true.  We skip the else blocks.
    ScopeEnum state;
    bool singleLine;
    
    std::map<std::string, Variable*> env;
    
    std::list<Token*> skipList; // Holds brackets while a block is being skipped

    Scope(bool isolated)
            : isolated(isolated)
            , state(NO_BLOCK)
            , singleLine(false)
    {}
    Scope(bool isolated, ScopeEnum state, bool singleLine)
            : isolated(isolated)
            , state(state)
            , singleLine(singleLine)
    {}

    Variable* getVar(const std::string& name)
    {
        if (env.find(name) == env.end())
            return NULL;
        return env[name];
    }

    void printEnv()
    {
        UI_debug_pile("Printing scope... %d variables.\n", env.size());
        for (std::map<std::string, Variable*>::iterator e = env.begin(); e != env.end(); e++)
        {
            UI_debug_pile(" '%s' has type '%s' with value '%s'\n", e->first.c_str(), e->second->getTypeString().c_str(), e->second->getValueString().c_str());
        }
    }
};


class Token
{
public:
    enum TokenEnum{NOT_A_TOKEN, VARIABLE, OPERATOR, SEPARATOR, KEYWORD};

    TokenEnum type;

    std::string text;
    Variable* var;

    OperatorEnum oper;
    int precedence;  // 1-15, lower is earlier
    bool associativeLeftToRight;
    
    SeparatorEnum sep;
    KeywordEnum keyword;

    // Null token
    Token()
            : type(NOT_A_TOKEN)
            , var(NULL)
            , oper(NOT_AN_OPERATOR), precedence(0), associativeLeftToRight(true)
            , sep(NOT_A_SEPARATOR)
            , keyword(KW_NONE)
    {}
    // Variable token
    Token(Variable* var, std::string text)
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
    Token(TokenEnum type, std::string text)
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
    
    void setKeyword(std::string Keyword)
    {
        keyword = getKeyword(Keyword);
    }
    
    void setOperator(std::string Oper)
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
    }
    
    void setSeparator(std::string s)
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


    std::string getTypeString()
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
    std::string getName()
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
};


class Outputter
{
    public:
    std::string syntax;
    
    Outputter(std::string syntax)
        : syntax(syntax)
    {}
    
    void error(std::string file, unsigned int line, std::string message)
    {
        // FIXME: Syntax must be in this order: file, line, message
        char buff[syntax.size() + file.size() + message.size() + 100];
        sprintf(buff, "%s", syntax.c_str());
        UI_error(buff, file.c_str(), line, message.c_str());
    }
};


inline void print_token_list(const std::list<Token>& ls)
{
    for(std::list<Token>::const_iterator e = ls.begin(); e != ls.end(); e++)
    {
        UI_print("%s\n", e->text.c_str());
    }
}


class Interpreter
{
private:

public:
    std::list<Scope> env;
    std::string currentFile;
    unsigned int lineNumber;
    bool errorFlag;
    Outputter outputter;
    
    Interpreter()
        : lineNumber(1)
        , errorFlag(false)
        //, outputter("%s: @%d:%s")
        , outputter("%s:%d:%s")
    {
        pushEnv(true);  // Push global scope
        
        // Add built-in functions
        Scope& s = *(env.begin());
        s.env["print"] = new Function(FN_PRINT);
        s.env["println"] = new Function(FN_PRINTLN);
        s.env["type"] = new Function(FN_TYPE);
        s.env["bool"] = new Function(FN_BOOL);
        s.env["int"] = new Function(FN_INT);
        s.env["float"] = new Function(FN_FLOAT);
        s.env["string"] = new Function(FN_STRING);
    }

    void printEnv()
    {
        UI_debug_pile("Printing Interpreter Environment... %d scopes.\n", env.size());
        for (std::list<Scope>::iterator e = env.begin(); e != env.end(); e++)
        {
            e->printEnv();
        }
    }
    
    void error(const char* formatted_text, ...);

    Variable* callFn(Function* fn, std::list<Token>& arguments)
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
        
        
        // Compare number of arguments
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

    Variable* evaluateExpression(Variable* A, OperatorEnum operation)
    {
        if (operation == NEGATE)
            return ::negate(A);
        //else if (operation == NOT)
        //    return ::not_(A);
        error("Error: Undefined operation\n");
        return A;
    }

    Variable* evaluateExpression(Variable* A, OperatorEnum operation, Variable* B)
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
        if (operation == SUBTRACT)
            return subtract(A, B);
        if (operation == MULTIPLY)
            return multiply(A, B);
        if (operation == DIVIDE)
            return divide(A, B);
        error("Error: Undefined operation\n");
        return A;
    }

    bool readFile(std::string filename);
    
    Variable* getArrayLiteral(std::list<Token>& tokens, std::list<Token>::iterator& e);

    bool defineClass(Variable* classvar, std::istream* stream);
    bool defineFunction(Variable* functionvar, std::istream* stream);
    Token evalTokens(std::list<Token>& tokens, bool beginning, bool wasTrueIf = false, bool wasFalseIf = false, bool subExpression = false);
    
    //bool interpret(std::string line);


    // Environment interface

    std::map<std::string, Variable*>* topEnv()
    {
        return &((env.begin())->env);
    }

    bool addVar(const std::string& name, Variable* var)
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

    bool setVar(const std::string& name, Variable* var)
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

    Variable* getVar(const std::string& name)
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

    void pushEnv(const Scope& scope)
    {
        env.push_front(scope);
    }

    void pushEnv(bool isolated)
    {
        env.push_front(Scope(isolated));
    }

    Scope& currentScope()
    {
        return (env.front());
    }

    void popEnv()
    {
        UI_debug_pile("Popping environment: %d scopes\n", env.size());
        if (env.size() > 1)
            env.erase(env.begin());
        UI_debug_pile("Popped environment: %d scopes\n", env.size());
    }

    void popAll()
    {
        while (env.size() > 1)
            env.erase(env.begin());
    }
};


#endif
