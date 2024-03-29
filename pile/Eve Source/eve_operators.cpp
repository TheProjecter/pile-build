/*
eve_operators.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the implementation for the operators and related functions.
*/

#include "eve_interpreter.h"

// FIXME: Replace Pile output with an error message system
#include "../pile_ui.h"

#include <cmath>
#include <string>
using namespace std;

extern Interpreter interpreter;


Bool* comparison(Variable* A, Variable* B, OperatorEnum oper)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in assignment.\n");
        return NULL;
    }
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
            switch(oper)
            {
                case EQUALS:
                    return new Bool(C->getValue() == D->getValue());
                case NOT_GREATER:
                case LESS_EQUAL:
                    return new Bool(C->getValue() <= D->getValue());
                case NOT_LESS:
                case GREATER_EQUAL:
                    return new Bool(C->getValue() >= D->getValue());
                case LESS:
                    return new Bool(C->getValue() < D->getValue());
                case GREATER:
                    return new Bool(C->getValue() > D->getValue());
                case NOT_EQUALS:
                    return new Bool(C->getValue() != D->getValue());
                default:
                    UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                    return NULL;
            }
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
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
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
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
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
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
            }
            else if(b == INT)
            {
                Int* D = static_cast<Int*>(B);
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
            }
            else
            {
                Float* D = static_cast<Float*>(B);
                switch(oper)
                {
                    case EQUALS:
                        return new Bool(C->getValue() == D->getValue());
                    case NOT_GREATER:
                    case LESS_EQUAL:
                        return new Bool(C->getValue() <= D->getValue());
                    case NOT_LESS:
                    case GREATER_EQUAL:
                        return new Bool(C->getValue() >= D->getValue());
                    case LESS:
                        return new Bool(C->getValue() < D->getValue());
                    case GREATER:
                        return new Bool(C->getValue() > D->getValue());
                    case AND:
                        return new Bool(C->getValue() && D->getValue());
                    case OR:
                        return new Bool(C->getValue() || D->getValue());
                    case NOT_EQUALS:
                        return new Bool(C->getValue() != D->getValue());
                    default:
                        UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                        return NULL;
                }
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
            switch(oper)
            {
                case EQUALS:
                    return new Bool(C->getValue() == D->getValue());
                case NOT_GREATER:
                case LESS_EQUAL:
                    return new Bool(C->getValue() <= D->getValue());
                case NOT_LESS:
                case GREATER_EQUAL:
                    return new Bool(C->getValue() >= D->getValue());
                case LESS:
                    return new Bool(C->getValue() < D->getValue());
                case GREATER:
                    return new Bool(C->getValue() > D->getValue());
                case NOT_EQUALS:
                    return new Bool(C->getValue() != D->getValue());
                default:
                    UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                    return NULL;
            }
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
                return NULL;
            }

            switch(oper)
            {
                case EQUALS:
                    return new Bool(C->getValue() == D->getValue());
                case NOT_GREATER:
                case LESS_EQUAL:
                    return new Bool(C->getValue() <= D->getValue());
                case NOT_LESS:
                case GREATER_EQUAL:
                    return new Bool(C->getValue() >= D->getValue());
                case LESS:
                    return new Bool(C->getValue() < D->getValue());
                case GREATER:
                    return new Bool(C->getValue() > D->getValue());
                case NOT_EQUALS:
                    return new Bool(C->getValue() != D->getValue());
                default:
                    UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                    return NULL;
            }
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
            switch(oper)
            {
                case EQUALS:
                    return new Bool(C->getValue() == D->getValue());
                case NOT_GREATER:
                case LESS_EQUAL:
                    return new Bool(C->getValue() <= D->getValue());
                case NOT_LESS:
                case GREATER_EQUAL:
                    return new Bool(C->getValue() >= D->getValue());
                case LESS:
                    return new Bool(C->getValue() < D->getValue());
                case GREATER:
                    return new Bool(C->getValue() > D->getValue());
                case NOT_EQUALS:
                    return new Bool(C->getValue() != D->getValue());
                default:
                    UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                    return NULL;
            }
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
            switch(oper)
            {
                case EQUALS:
                    return new Bool(C->getValue() == D->getValue());
                case NOT_GREATER:
                case LESS_EQUAL:
                    return new Bool(C->getValue() <= D->getValue());
                case NOT_LESS:
                case GREATER_EQUAL:
                    return new Bool(C->getValue() >= D->getValue());
                case LESS:
                    return new Bool(C->getValue() < D->getValue());
                case GREATER:
                    return new Bool(C->getValue() > D->getValue());
                case NOT_EQUALS:
                    return new Bool(C->getValue() != D->getValue());
                default:
                    UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                    return NULL;
            }
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
            switch(oper)
            {
                case EQUALS:
                    return new Bool(C->getValue() == D->getValue());
                case NOT_GREATER:
                case LESS_EQUAL:
                    return new Bool(C->getValue() <= D->getValue());
                case NOT_LESS:
                case GREATER_EQUAL:
                    return new Bool(C->getValue() >= D->getValue());
                case LESS:
                    return new Bool(C->getValue() < D->getValue());
                case GREATER:
                    return new Bool(C->getValue() > D->getValue());
                case NOT_EQUALS:
                    return new Bool(C->getValue() != D->getValue());
                default:
                    UI_debug_pile("Pile Error: Bad operator passed to comparison().\n");
                    return NULL;
            }
        }
    }

    //if(mismatch)
    {
        interpreter.error("Error: Types do not match in assignment: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return NULL;
    }
    return NULL;
}


Variable* assign(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in assignment.\n");
        return NULL;
    }
    if(!A->reference)
    {
        interpreter.error("Error: Assigning value to a non-reference variable.\n");
        return NULL;
    }

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
            b = D->getValueType();
            if(a != b)
            {
                interpreter.error("Error: Types do not match in assignment: Array<%s> vs Array<%s>\n", C->getValueTypeString().c_str(), D->getValueTypeString().c_str());
                return NULL;
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
        return NULL;
    }
    return A;
}


Variable* add_assign(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in assignment.\n");
        return NULL;
    }
    if(!A->reference)
    {
        interpreter.error("Error: Assigning value to a non-reference variable.\n");
        return NULL;
    }

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
        return NULL;
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
                return NULL;
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
        return NULL;
    }
    else if(a == PROCEDURE)
    {
        interpreter.error("Error: Addition operation not defined for type 'procedure'.\n");
        return NULL;
    }

    if(mismatch)
    {
        interpreter.error("Error: Types do not match in assignment: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return NULL;
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

Variable* modulus_assign(Variable* A, Variable* B)
{
    Variable* C = ::modulus(A, B);
    C = assign(A, C);
    return C;
}

Variable* exponentiate_assign(Variable* A, Variable* B)
{
    Variable* C = exponentiate(A, B);
    C = assign(A, C);
    return C;
}

Variable* add(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in addition.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a != b && !(a == FLOAT && b == INT) && !(b == FLOAT && a == INT))
    {
        interpreter.error("Error: Types do not match in addition: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return NULL;
    }
    if(a == STRING)
    {
        String* C = static_cast<String*>(A);
        String* D = static_cast<String*>(B);
        String* R = new String("<temp>");
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
        return NULL;
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
            return NULL;
        }

        vector<Variable*> va = C->getValue();
        vector<Variable*>& vb = D->getValue();

        for(vector<Variable*>::iterator e = vb.begin(); e != vb.end(); e++)
        {
            va.push_back(*e);
        }

        Array* arr = new Array("<temp>", va, a);

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

        List* lst = new List("<temp>", va);

        return lst;
    }
    else if(a == FUNCTION)
    {
        Function* C = static_cast<Function*>(A);
        Function* D = static_cast<Function*>(B);
        Function* R = new Function("<temp>", FN_NONE);
        R->setValue(C->getValue() + D->getValue());
        return R;
    }
    else if(a == PROCEDURE)
    {
        Procedure* C = static_cast<Procedure*>(A);
        Procedure* D = static_cast<Procedure*>(B);
        Procedure* R = new Procedure("<temp>");
        R->setValue(C->getValue() + D->getValue());
        return R;
    }
    return A;
}

Variable* subtract(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in subtraction.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a != b && !(a == FLOAT && b == INT) && !(b == FLOAT && a == INT))
    {
        interpreter.error("Error: Types do not match in subtraction: %s vs %s\n", A->getTypeString().c_str(), B->getTypeString().c_str());
        return NULL;
    }
    if(a == STRING)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'string'.\n");
        return NULL;
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
        return NULL;
    }
    else if(a == MACRO)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'macro'.\n");
        return NULL;
    }
    else if(a == ARRAY)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'array'.\n");
        return NULL;
    }
    else if(a == LIST)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'list'.\n");
        return NULL;
    }
    else if(a == FUNCTION)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'function'.\n");
        return NULL;
    }
    else if(a == PROCEDURE)
    {
        interpreter.error("Error: Subtraction operation not defined for type 'procedure'.\n");
        return NULL;
    }
    return A;
}

Variable* multiply(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in multiplication.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE
      || b == STRING || b == BOOL || b == MACRO || b == ARRAY || b == LIST || b == FUNCTION || b == PROCEDURE)
    {
        interpreter.error("Error: Multiplication not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return NULL;
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
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in division.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE
      || b == STRING || b == BOOL || b == MACRO || b == ARRAY || b == LIST || b == FUNCTION || b == PROCEDURE)
    {
        interpreter.error("Error: Division not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return NULL;
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

Variable* modulus(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in modulus.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE
      || b == STRING || b == BOOL || b == MACRO || b == ARRAY || b == LIST || b == FUNCTION || b == PROCEDURE)
    {
        interpreter.error("Error: Modulus not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return NULL;
    }
    if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Int* R = new Int;
            R->setValue(C->getValue() % D->getValue());
            return R;
        }
        else if(b == FLOAT)
        {
            interpreter.error("Error: Modulus not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
            return NULL;
        }
    }
    else if(a == FLOAT)
    {
        if(b == INT)
        {
            interpreter.error("Error: Modulus not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
            return NULL;
        }
        else if(b == FLOAT)
        {
            interpreter.error("Error: Modulus not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
            return NULL;
        }
    }
    return A;
}

Variable* exponentiate(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in exponentiation.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE
      || b == STRING || b == BOOL || b == MACRO || b == ARRAY || b == LIST || b == FUNCTION || b == PROCEDURE)
    {
        interpreter.error("Error: Exponentiation not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return NULL;
    }
    if(a == INT)
    {
        Int* C = static_cast<Int*>(A);
        if(b == INT)
        {
            Int* D = static_cast<Int*>(B);
            Int* R = new Int;
            R->setValue(pow(float(C->getValue()), D->getValue()));
            return R;
        }
        else if(b == FLOAT)
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(pow(C->getValue(), D->getValue()));
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
            R->setValue(pow(C->getValue(), D->getValue()));
            return R;
        }
        else if(b == FLOAT)
        {
            Float* D = static_cast<Float*>(B);
            Float* R = new Float;
            R->setValue(pow(C->getValue(), D->getValue()));
            return R;
        }
    }
    return A;
}

Variable* dot(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in dot operation.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(B->literal)
    {
        interpreter.error("Error: Literal variable used in 'dot' operation\n");
        return NULL;
    }
    if(a == CLASS_OBJECT)
    {
        ClassObject* C = static_cast<ClassObject*>(A);
        Variable* result = C->getVariable(B->text);
        if(result == NULL)
            interpreter.error("Error: %s is not a member of %s.\n", B->text.c_str(), C->className.c_str());
        return result;
    }
    else if(a == ARRAY)
    {
        Array* C = static_cast<Array*>(A);
        Variable* result = NULL;
        if(B->text == "size")
        {
            interpreter.array_size->parentObject = A;
            result = interpreter.array_size;
        }
        if(result == NULL)
            interpreter.error("Error: %s is not a member of %s.\n", B->text.c_str(), C->text.c_str());
        return result;
    }

    interpreter.error("Error: 'Dot' not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
    return NULL;
}

Variable* array_access(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in array_access operation.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a != ARRAY || b != INT)
    {
        interpreter.error("Error: Array access not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return NULL;
    }
    Array* C = static_cast<Array*>(A);
    Int* D = static_cast<Int*>(B);
    vector<Variable*>& v = C->getValue();
    Variable* result = NULL;
    int index = D->getValue();

    if(index >= 0 && (unsigned int)index < v.size())
        result = v[index];

    if(result == NULL)
        interpreter.error("Error: Array index out of bounds.\n");
    return result;
}

Bool* has_element(Variable* A, Variable* B)
{
    if(A == NULL || B == NULL)
    {
        interpreter.error("Error: Void variable in has_element operation.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    TypeEnum b = B->getType();
    if(a != ARRAY && b != ARRAY)
    {
        interpreter.error("Error: Has_element not defined for types '%s' and '%s'\n", getTypeString(a).c_str(), getTypeString(b).c_str());
        return NULL;
    }

    Array* C;
    Variable* other;
    TypeEnum otherType;
    if(a == ARRAY)
    {
        C = static_cast<Array*>(A);
        other = B;
        otherType = b;
    }
    else
    {
        C = static_cast<Array*>(B);
        other = A;
        otherType = a;
    }

    if(otherType != C->getValueType())
    {
        interpreter.error("Error: Has_element not defined for types '%s' and '%s'\n", getTypeString(ARRAY).c_str(), getTypeString(otherType).c_str());
        return NULL;
    }

    vector<Variable*>& v = C->getValue();
    for(vector<Variable*>::iterator e = v.begin(); e != v.end(); e++)
    {
        Bool* test = comparison(other, *e, EQUALS);
        if(test->getValue() == true)
            return test;
        delete test;
    }

    return new Bool(false);
}


Bool* not_has_element(Variable* A, Variable* B)
{
    Bool* b = has_element(A, B);
    if(b == NULL)
        return NULL;
    b->setValue(!b->getValue());
    return b;
}


Variable* negate(Variable* A)
{
    if(A == NULL)
    {
        interpreter.error("Error: Void variable in negation.\n");
        return NULL;
    }
    TypeEnum a = A->getType();
    if(a == STRING || a == BOOL || a == MACRO || a == ARRAY || a == LIST || a == FUNCTION || a == PROCEDURE)
    {
        interpreter.error("Error: Negation not defined for type '%s'\n", getTypeString(a).c_str());
        return NULL;
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
