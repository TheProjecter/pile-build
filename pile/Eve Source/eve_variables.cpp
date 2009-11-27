/*
eve_variables.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the implementation of the Variable classes and related
functions.
*/

#include "eve_interpreter.h"
// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"

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










Variable::Variable(TypeEnum type, const std::string& text)
        : type(type)
        , text(text)
        , literal(false)
        , temp(false)
        , reference(false)
{}
TypeEnum Variable::getType()
{
    return type;
}
// This could easily be a virtual function instead... and then it'd be independent of some external stuff.
std::string Variable::getTypeString()
{
    return ::getTypeString(type);
}
Variable* Variable::makeTemp()
{
    temp = true;
    return this;
}






TypeName::TypeName(const TypeName& typeName)
        : Variable(TYPENAME, ::getTypeString(typeName.value))
        , value(typeName.value)
        , subType(typeName.subType)
{
        literal = typeName.literal;
        temp = typeName.temp;
        reference = typeName.reference;
}
TypeName::TypeName(TypeEnum type)
        : Variable(TYPENAME, ::getTypeString(type))
        , value(type)
        , subType(NOT_A_TYPE)
{}
TypeName::TypeName(const std::string& text, TypeEnum type)
        : Variable(TYPENAME, text)
        , value(type)
        , subType(NOT_A_TYPE)
{}
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

void TypeName::setValue(const TypeEnum& val)
{
    value = val;
}
TypeEnum& TypeName::getValue()
{
    return value;
}
std::string TypeName::getValueString()
{
    return ::getTypeString(value);
}
Variable* TypeName::copy()
{
    Variable* cp = new TypeName(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}






Void::Void(const std::string& val)
        : Variable(VOID, val)
        , value(val)
{}
void Void::setValue(const std::string& val)
{
    value = val;
}
std::string& Void::getValue()
{
    return value;
}
std::string Void::getValueString()
{
    return value;
}
Variable* Void::copy()
{
    Variable* cp = new Void(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}
    
    



String::String(const std::string& text)
        : Variable(STRING, text)
{}
String::String(const std::string& text, const std::string& value)
        : Variable(STRING, text)
        , value(value)
{}
void String::setValue(const std::string& val)
{
    value = val;
}
std::string& String::getValue()
{
    return value;
}
std::string String::getValueString()
{
    return value;
}
Variable* String::copy()
{
    Variable* cp = new String(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}











Bool::Bool(bool value)
        : Variable(BOOL, "<temp>")
        , value(value)
{}
Bool::Bool(const std::string& text, bool value)
        : Variable(BOOL, text)
        , value(value)
{}
bool& Bool::getValue()
{
    return value;
}
void Bool::setValue(const bool& val)
{
    value = val;
}
std::string Bool::getValueString()
{
    return (value? "true" : "false");
}
Variable* Bool::copy()
{
    Variable* cp = new Bool(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}







Int::Int()
        : Variable(INT, "<temp>")
        , value(0)
{}
Int::Int(const std::string& text, int value)
        : Variable(INT, text)
        , value(value)
{}
int& Int::getValue()
{
    return value;
}
void Int::setValue(const int& val)
{
    value = val;
}
std::string Int::getValueString()
{
    char buff[20];
    sprintf(buff, "%d", value);
    return buff;
}
Variable* Int::copy()
{
    Variable* cp = new Int(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}







Float::Float()
        : Variable(FLOAT, "<temp>")
        , value(0.0f)
{}
Float::Float(const std::string& text, float value)
        : Variable(FLOAT, text)
        , value(value)
{}
float& Float::getValue()
{
    return value;
}
void Float::setValue(const float& val)
{
    value = val;
}
std::string Float::getValueString()
{
    char buff[200];
    sprintf(buff, "%f", value);
    return buff;
}
Variable* Float::copy()
{
    Variable* cp = new Float(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}





Macro::Macro()
        : Variable(MACRO, "<temp>")
{}
Macro::Macro(const std::string& text)
        : Variable(MACRO, text)
{}
std::string& Macro::getValue()
{
    return value;
}
void Macro::setValue(const std::string& val)
{
    value = val;
}
std::string Macro::getValueString()
{
    return value;
}
Variable* Macro::copy()
{
    Variable* cp = new Macro(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}






Variable* Array::size_fn(Variable* arr)
{
    if(arr == NULL || arr->getType() != ARRAY)
        return NULL;
    
    return new Int("<temp>", static_cast<Array*>(arr)->size());
}

Array::Array(const std::string& text, TypeEnum valueType)
        : Variable(ARRAY, text)
        , valueType(valueType)
{}
Array::Array(const std::string& text, const std::vector<Variable*>& value, TypeEnum valueType)
        : Variable(ARRAY, text)
        , valueType(valueType)
        , value(value)
{
    for(unsigned int i = 0; i < value.size(); i++)
    {
        value[i]->reference = true;
    }
}
std::vector<Variable*>& Array::getValue()
{
    return value;
}
void Array::setValue(const std::vector<Variable*>& val)
{
    value = val;  // Needs copy
}
void Array::setValueType(TypeEnum newValueType)
{
    valueType = newValueType;
}
std::string Array::getValueTypeString()
{
    return ::getTypeString(valueType);
}
TypeEnum Array::getValueType()
{
    return valueType;
}
std::string Array::getValueString()
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
void Array::push_back(Variable* var)
{
    if(var != NULL && var->getType() == valueType)
    {
        Variable* v = (var)->copy();
        v->reference = true;
        value.push_back(v);
    }
}
void Array::push_back(const std::vector<Variable*>& vec)
{
    for(std::vector<Variable*>::const_iterator e = vec.begin(); e != vec.end(); e++)
    {
        if((*e)->getType() == valueType)
        {
            Variable* v = (*e)->copy();
            v->reference = true;
            value.push_back(v);
        }
    }
}

unsigned int Array::size()
{
    return value.size();
}

Variable* Array::copy()
{
    Variable* cp = new Array(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}








List::List(const std::string& text)
        : Variable(LIST, text)
{}
List::List(const std::string& text, const std::list<Variable*>& value)
        : Variable(LIST, text)
        , value(value)
{}
std::list<Variable*>& List::getValue()
{
    return value;
}
void List::setValue(const std::list<Variable*>& val)
{
    value = val;  // Needs copy
}
std::string List::getValueString()
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
void List::push_back(Variable* var)
{
    if(var != NULL)
        value.push_back(var);
}
Variable* List::copy()
{
    Variable* cp = new List(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}









Procedure::Procedure(const std::string& text)
        : Variable(PROCEDURE, text)
{}
std::string& Procedure::getValue()
{
    return value;
}
void Procedure::setValue(const std::string& val)
{
    value = val;
}
std::string Procedure::getValueString()
{
    return value;
}
Variable* Procedure::copy()
{
    Variable* cp = new Procedure(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}








Class::Class(const std::string& name)
        : Variable(CLASS, name)
        , name(name)
{}
void Class::addVariable(const std::string& vartype, const std::string& varname)
{
    vars.push_back(VarRecord(vartype, varname));
}
void Class::addFunction(std::string name, Function* f)
{
    //vars.push_back(VarRecord(varname, argtypes, definition));
    if(f != NULL)
    {
        f->makeAsMethod(this->name);
        vars.push_back(VarRecord(name, f));
    }
}
std::string Class::getValueString()
{
    return name;
}
Variable* Class::copy()
{
    Variable* cp = new Class(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
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


Variable* ClassObject::getVariable(const std::string& var)
{
    /*UI_print("Searching for: %s\n", var.c_str());
    for(std::map<std::string, Variable*>::iterator f = vars.begin(); f != vars.end(); f++)
    {
        UI_print("Name: %s\n", f->first.c_str());
    }*/
    std::map<std::string, Variable*>::iterator e = vars.find(var);
    if(e == vars.end())
        return NULL;
    // Found it
    if(e->second != NULL)
        UI_debug_pile("Returning member of type: %s\n", e->second->getTypeString().c_str());
    else
        UI_debug_pile("Returning NULL member\n");
    if(e->second->getType() == FUNCTION)
    {
        Function* f = static_cast<Function*>(e->second);
        f->parentObject = this;
    }
    return e->second;
}

std::string ClassObject::getValueString()
{
    return name;
}

Variable* ClassObject::copy()
{
    Variable* cp = new ClassObject(*this);
    cp->temp = false;
    cp->reference = false;
    return cp;
}

