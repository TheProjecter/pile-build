/*
eve_interpreter.h

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

Eve, an interpreted programming language.
This file contains the class definitions for the interpreter.
*/



#ifndef _EVE_INTERPRETER_H__
#define _EVE_INTERPRETER_H__

// FIXME: Replace Pile output with an error message system
//#include "../pile_ui.h"
#include <list>
#include <vector>
#include <map>
#include <string>

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

enum OperatorEnum{NOT_AN_OPERATOR, ADD, SUBTRACT, NEGATE, ASSIGN, ADD_ASSIGN, SUBTRACT_ASSIGN, MULTIPLY_ASSIGN, DIVIDE_ASSIGN, MODULUS_ASSIGN, EXPONENTIATE_ASSIGN, 
                  MULTIPLY, DIVIDE, MODULUS, EXPONENTIATE, EQUALS, NOT_EQUALS, LESS, GREATER, NOT_LESS, NOT_GREATER, LESS_EQUAL,
                  GREATER_EQUAL, NOT, AND, OR, CALL, CONTINUATION, COLON, DOT, BITWISE_AND, BITWISE_XOR, BITWISE_OR, ARRAY_ACCESS, HAS_ELEMENT, NOT_HAS_ELEMENT
                 };
                 
enum SeparatorEnum{NOT_A_SEPARATOR, COMMA, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, 
                OPEN_SQUARE_BRACKET, CLOSE_SQUARE_BRACKET, OPEN_CURLY_BRACKET, 
                CLOSE_CURLY_BRACKET, SEMICOLON
                 };

enum KeywordEnum{KW_NONE, KW_IF, KW_ELSE, KW_RETURN};

enum FunctionEnum{FN_NONE, FN_EXTERNAL, FN_PRINT, FN_PRINTLN, FN_WARNING, FN_ERROR, FN_DEBUG, FN_TYPE, FN_STRING, FN_BOOL, FN_INT, FN_FLOAT, FN_INCLUDE, FN_LS, FN_DEFINED, FN_COPY, FN_MOVE, FN_DELETE, FN_MKDIR, FN_MKPATH, FN_MKFILE, FN_CHMOD, FN_MOD_TIME};

KeywordEnum getKeyword(const std::string& str);

class Token;
std::list<Token> tokenize1(std::string& line, bool& continuation);

std::string getTypeString(TypeEnum type);
std::string getOperatorString(OperatorEnum type);
std::string getSeparatorString(SeparatorEnum type);

class Interpreter;
class Variable;
class Bool;
class String;
class Array;
class Function;
class ClassObject;
Variable* callBuiltIn(FunctionEnum fn, std::vector<Variable*>& args);
Bool* boolCast(Variable* v);


String* convertArg_String(Variable* arg);
Array* convertArg_Array(Variable* arg, TypeEnum valueType);
ClassObject* convertArg_ClassObject(Variable* arg, const std::string& className);


bool isConvertable(TypeEnum source, TypeEnum dest);

class Variable
{
protected:
    TypeEnum type;
public:
    std::string text;
    bool literal;
    bool temp;
    bool reference;  // Used as a message to callFn that this should be passed by reference.
    Variable(TypeEnum type, const std::string& text);
    TypeEnum getType();
    virtual Variable* copy() = 0;
    virtual std::string getValueString() = 0;
    // This could easily be a virtual function instead... and then it'd be independent of some external stuff.
    std::string getTypeString();
    Variable* makeTemp();
};

// This class represents a type declaration.
class TypeName : public Variable
{
private:
    TypeEnum value; // What type is being declared?
public:
    TypeEnum subType; // Used for arrays
    TypeName(const TypeName& typeName);
    TypeName(TypeEnum type);
    TypeName(const std::string& text, TypeEnum type);
    TypeName(const std::string& text, Variable* var);
    
    void setValue(const TypeEnum& val);
    TypeEnum& getValue();
    virtual std::string getValueString();
    virtual Variable* copy();
};

// This class represents an undefined variable.
class Void : public Variable
{
private:
    std::string value; // What type is being declared?
public:
    Void(const std::string& val);
    void setValue(const std::string& val);
    std::string& getValue();
    virtual std::string getValueString();
    virtual Variable* copy();
};

class String : public Variable
{
private:
    std::string value;
public:
    String(const std::string& text);
    String(const std::string& text, const std::string& value);
    void setValue(const std::string& val);
    std::string& getValue();
    virtual std::string getValueString();
    virtual Variable* copy();
};

class Bool : public Variable
{
private:
    bool value;
public:
    Bool(bool value);
    Bool(const std::string& text, bool value);
    bool& getValue();
    void setValue(const bool& val);
    virtual std::string getValueString();
    virtual Variable* copy();
};

class Int : public Variable
{
private:
    int value;
public:
    Int();
    Int(const std::string& text, int value);
    int& getValue();
    void setValue(const int& val);
    virtual std::string getValueString();
    virtual Variable* copy();
};

class Float : public Variable
{
private:
    float value;
public:
    Float();
    Float(const std::string& text, float value);
    float& getValue();
    void setValue(const float& val);
    virtual std::string getValueString();
    virtual Variable* copy();
};

class Macro : public Variable
{
private:
    std::string value;
public:
    Macro();
    Macro(const std::string& text);
    std::string& getValue();
    void setValue(const std::string& val);
    virtual std::string getValueString();
    virtual Variable* copy();
};



class Array : public Variable
{
private:
    TypeEnum valueType;
    std::vector<Variable*> value;
public:
    static Variable* size_fn(Variable* arr);

    Array(const std::string& text, TypeEnum valueType);
    Array(const std::string& text, const std::vector<Variable*>& value, TypeEnum valueType);
    std::vector<Variable*>& getValue();
    void setValue(const std::vector<Variable*>& val);
    void setValueType(TypeEnum newValueType);
    std::string getValueTypeString();
    TypeEnum getValueType();
    virtual std::string getValueString();
    void push_back(Variable* var);
    void push_back(const std::vector<Variable*>& vec);
    
    unsigned int size();
    
    virtual Variable* copy();
};

class List : public Variable
{
private:
    std::list<Variable*> value;
public:
    List(const std::string& text);
    List(const std::string& text, const std::list<Variable*>& value);
    std::list<Variable*>& getValue();
    void setValue(const std::list<Variable*>& val);
    virtual std::string getValueString();
    void push_back(Variable* var);
    virtual Variable* copy();
};

class Function : public Variable
{
private:
    std::string value;  // Definition
    std::vector<TypeName> argt;  // Types of arguments
    std::vector<std::string> args;  // Names of arguments
    FunctionEnum builtIn;
    Variable* (*external_fn0)();
    Variable* (*external_fn1)(Variable*);
    Variable* (*external_fn2)(Variable*, Variable*);
    Variable* (*external_fn3)(Variable*, Variable*, Variable*);
    Variable* (*external_fn4)(Variable*, Variable*, Variable*, Variable*);
    Variable* (*external_fn5)(Variable*, Variable*, Variable*, Variable*, Variable*);
public:
    TypeName returnType;

    std::string definitionFile;
    unsigned int lineNumber;
    bool isMethod;
    Variable* parentObject;


    Function(const std::string& text, const std::vector<TypeName>& argt, const std::string& value);
    Function(const std::string& text, Variable* (*external_fn)());
    Function(const std::string& text, Variable* (*external_fn)(Variable*));
    Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*));
    Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*, Variable*));
    Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*, Variable*, Variable*));
    Function(const std::string& text, Variable* (*external_fn)(Variable*, Variable*, Variable*, Variable*, Variable*));
    Function(const std::string& text, FunctionEnum builtIn);
    Function(const std::string& text, const Function& fn);
    
    std::string& getValue();
    std::vector<TypeName>& getArgTypes();
    void setValue(const std::string& val);
    void setArgTypes(const std::vector<TypeName>& argTypes);
    void addArg(const TypeName& argType, const std::string& argName);
    /*void loadFromSig(const std::list<Token>& fnSignature)
    {
        // ...
    }*/
    // Convert into a class method
    void makeAsMethod(std::string className);
    virtual std::string getValueString();
    bool isBuiltIn();
    FunctionEnum getBuiltIn();
    Variable* call(Interpreter& interpreter, std::vector<Variable*>& args);
    
    virtual Variable* copy();
};

class Procedure : public Variable
{
private:
    std::string value;
public:
    Procedure(const std::string& text);
    std::string& getValue();
    void setValue(const std::string& val);
    virtual std::string getValueString();
    virtual Variable* copy();
};



class Class : public Variable
{
public:
    std::string name;
    
    class VarRecord
    {
        public:
        std::string type;
        std::string name;
        Function* fn;
        
        VarRecord(const std::string& type, const std::string& name)
            : type(type), name(name), fn(NULL)
        {}
        /*VarRecord(const std::string& name, const std::list<std::string> argtypes, const std::string& definition)
            : type("function"), name(name), functionArgs(argtypes), functionDefinition(definition)
        {}*/
        VarRecord(const std::string& name, Function* f)
            : type("function"), name(name), fn(f)
        {}
    };
    std::list<VarRecord> vars;
    
    Class(const std::string& name);
    void addVariable(const std::string& vartype, const std::string& varname);
    void addFunction(std::string name, Function* f);
    virtual std::string getValueString();
    virtual Variable* copy();
};

class ClassObject : public Variable
{
private:
    
    
public:
    std::string name;
    std::map<std::string, Variable*> vars;
    std::string className;
    ClassObject(const std::string& text, const std::string& name);
    
    Variable* getVariable(const std::string& var);
    virtual std::string getValueString();
    virtual Variable* copy();
};


Bool* comparison(Variable* A, Variable* B, OperatorEnum oper);

Variable* assign(Variable* A, Variable* B);

Variable* add(Variable* A, Variable* B);

Variable* subtract(Variable* A, Variable* B);

Variable* negate(Variable* A);

Variable* multiply(Variable* A, Variable* B);

Variable* divide(Variable* A, Variable* B);

Variable* modulus(Variable* A, Variable* B);

Variable* exponentiate(Variable* A, Variable* B);

Variable* dot(Variable* A, Variable* B);

Variable* array_access(Variable* A, Variable* B);
Bool* has_element(Variable* A, Variable* B);
Bool* not_has_element(Variable* A, Variable* B);

Variable* add_assign(Variable* A, Variable* B);
Variable* subtract_assign(Variable* A, Variable* B);
Variable* multiply_assign(Variable* A, Variable* B);
Variable* divide_assign(Variable* A, Variable* B);
Variable* modulus_assign(Variable* A, Variable* B);

Variable* exponentiate_assign(Variable* A, Variable* B);

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

    Scope(bool isolated);
    Scope(bool isolated, ScopeEnum state, bool singleLine);

    Variable* getVar(const std::string& name);

    void printEnv();
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
    Token();
    // Variable token
    Token(Variable* var, std::string text);
    // Operator, Separator, or Keyword
    Token(TokenEnum type, std::string text);
    
    void setKeyword(std::string Keyword);
    
    void setOperator(std::string Oper);
    
    void setSeparator(std::string s);


    std::string getTypeString();
    std::string getName();
    
};


class Outputter
{
    public:
    std::string syntax;
    
    Outputter(std::string syntax);
    
    void setSyntax(std::string syn);
    
    void error(std::string file, unsigned int line, std::string message);
};


/*inline void print_token_list(const std::list<Token>& ls)
{
    for(std::list<Token>::const_iterator e = ls.begin(); e != ls.end(); e++)
    {
        UI_print("%s\n", e->text.c_str());
    }
}*/


class Interpreter
{
private:

public:
    std::list<Scope> env;
    // FIXME: Move this to Scope!!!
    std::list<Class*> classDefs;
    std::string currentFile;
    unsigned int lineNumber;
    bool errorFlag;
    Outputter outputter;
    
    Function* array_size;
    bool allowDeclarations;
    
    Interpreter();
    
    void addClass(Class* c);

    void printEnv();
    
    void error(const char* formatted_text, ...);

    Variable* callFn(Function* fn, std::list<Token>& arguments);
    
    Variable* evaluateExpression(Variable* A, OperatorEnum operation);

    Variable* evaluateExpression(Variable* A, OperatorEnum operation, Variable* B);
    
    bool readFile(std::string filename);
    
    Variable* getArrayLiteral(std::list<Token>& tokens, std::list<Token>::iterator& e);

    bool defineClass(Variable* classvar, std::istream* stream);
    bool defineFunction(Variable* functionvar, std::istream* stream);
    Token evalTokens(std::list<Token>& tokens, bool beginning, bool wasTrueIf = false, bool wasFalseIf = false, bool subExpression = false);
    
    //bool interpret(std::string line);


    // Environment interface

    std::map<std::string, Variable*>* topEnv();

    bool addVar(const std::string& name, Variable* var);

    bool setVar(const std::string& name, Variable* var);

    Variable* getVar(const std::string& name);

    void pushEnv(const Scope& scope);

    void pushEnv(bool isolated);

    Scope& currentScope();

    void popEnv();

    void popAll();
    
    void reset();
};


#endif
