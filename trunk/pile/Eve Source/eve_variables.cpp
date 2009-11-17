#include "eve_interpreter.h"

#include <string>
using namespace std;

extern Interpreter interpreter;

TypeEnum getTypeFromString(const string& str);


string getTypeString(TypeEnum type)
{
    switch(type)
    {
        case VOID:
            return "void";
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
        case CLASS:
            return "class";
        case CLASS_OBJECT:
            return "class_object";
        case NOT_A_TYPE:
            return "not_a_type";
        default:
            return "unknown";
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
        case MODULUS_ASSIGN:
            return "%=";
        case EXPONENTIATE_ASSIGN:
            return "**=";
        case MULTIPLY:
            return "*";
        case DIVIDE:
            return "/";
        case MODULUS:
            return "%";
        case EXPONENTIATE:
            return "**";
        case EQUALS:
            return "==";
        case NOT_EQUALS:
            return "!=";
        case LESS:
            return "<";
        case GREATER:
            return ">";
        case NOT_LESS:
            return "!<";
        case NOT_GREATER:
            return "!>";
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
        case DOT:
            return ".";
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
        case ARRAY_ACCESS:
            return "[]";
        case HAS_ELEMENT:
            return "<>";
        case NOT_HAS_ELEMENT:
            return "><";
        default:
            return "N/A";
    }
}




bool isConvertable(TypeEnum source, TypeEnum dest)
{
    if(source == dest)
        return true;
    if(source == BOOL)
    {
        return (dest == INT || dest == FLOAT);
    }
    if(source == INT)
    {
        return (dest == BOOL || dest == FLOAT);
    }
    if(source == FLOAT)
    {
        return (dest == BOOL || dest == INT);
    }
    if(source == STRING)
    {
        return (dest == STRING);
    }
    // FIXME: Finish this.
    return false;
}


TypeName::TypeName(const std::string& text, Variable* var)
        : Variable(TYPENAME, text)
        , value(NOT_A_TYPE)
        , subType(NOT_A_TYPE)
{
    if(var != NULL)
    {
        this->text = var->getTypeString();
        value = var->getType();
        if(value == ARRAY)
            subType = static_cast<Array*>(var)->getValueType();
    }
}


ClassObject::ClassObject(const std::string& text, const std::string& name)
    : Variable(CLASS_OBJECT, text)
    , name(name)
{
    
    // Search the registered classes for the name
    // If it's found, create all new variables.
    // If it's not found, set className to ""
    for(std::list<Class*>::iterator e = interpreter.classDefs.begin(); e != interpreter.classDefs.end(); e++)
    {
        if((*e)->name == name)
        {
            // Found it.  Create the variables.
            Class* def = *e;
            className = name;
            for(std::list<Class::VarRecord>::iterator r = def->vars.begin(); r != def->vars.end(); r++)
            {
                TypeEnum t = getTypeFromString(r->type);
                Variable* v = NULL;
                switch(t)
                {
                    case VOID:
                        interpreter.error("Member type 'void' is not allowed.\n");
                        break;
                    case BOOL:
                        v = new Bool(r->name, false);
                        break;
                    case INT:
                        v = new Int(r->name, 0);
                        break;
                    case FLOAT:
                        v = new Float(r->name, 0.0f);
                        break;
                    case STRING:
                        v = new String(r->name, "");
                        break;
                    case MACRO:
                        v = new Macro(r->name);
                        break;
                    case ARRAY:
                        v = new Array(r->name, NOT_A_TYPE);
                        break;
                    case LIST:
                        v = new List(r->name);
                        break;
                    case FUNCTION:
                        if(r->fn != NULL)
                        {
                            Function* f = new Function(r->name, *(r->fn));
                            v = f;
                        }
                        break;
                    default:
                        // FIXME
                        interpreter.error("Member type '%s' not implemented yet.\n", r->type.c_str());
                        break;
                }
                if(v != NULL)
                {
                    v->reference = true;
                    vars.insert(make_pair(r->name, v));
                }
            }
            return;
        }
    }
}
