/////////////////////////////////////////////////////////////////////////////////
// Copyright 2021 Guillaume Guillet                                            //
//                                                                             //
// Licensed under the Apache License, Version 2.0 (the "License");             //
// you may not use this file except in compliance with the License.            //
// You may obtain a copy of the License at                                     //
//                                                                             //
//     http://www.apache.org/licenses/LICENSE-2.0                              //
//                                                                             //
// Unless required by applicable law or agreed to in writing, software         //
// distributed under the License is distributed on an "AS IS" BASIS,           //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.    //
// See the License for the specific language governing permissions and         //
// limitations under the License.                                              //
/////////////////////////////////////////////////////////////////////////////////

#include "C_instruction.hpp"
#include "C_readableBus.hpp"
#include "C_compilerData.hpp"
#include "C_console.hpp"
#include "C_keyword.hpp"
#include "C_bus.hpp"
#include <algorithm>

namespace codeg
{

Instruction::Instruction(){}
Instruction::~Instruction(){}

///Instruction_set
Instruction_set::Instruction_set(){}
Instruction_set::~Instruction_set(){}

std::string Instruction_set::getName() const
{
    return "set";
}

void Instruction_set::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 3 )
    {//Check size
        throw codeg::CompileError("set : bad arguments size (wanted 3 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {//Check name
        throw codeg::CompileError("set : bad argument (argument 1 [name] must be a valid name)");
    }

    codeg::Keyword argString;
    if ( !argString.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check string
        throw codeg::CompileError("set : bad argument (argument 2 [string] must be a valid string)");
    }

    if ( codeg::MacroCheck(data._macros, argName._str) )
    {//Check if already set
        data._macros[argName._str] = argString._str;
        codeg::ConsoleWrite("[warning] set : macro \""+ argName._str +"\" already exist and will be replaced");
    }
    else
    {
        data._macros[argName._str] = argString._str;
    }
}

///Instruction_var
Instruction_var::Instruction_var(){}
Instruction_var::~Instruction_var(){}

std::string Instruction_var::getName() const
{
    return "var";
}

void Instruction_var::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() == 3 )
    {//Check size
        codeg::Keyword argVarName;
        if ( !argVarName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check var name
            throw codeg::CompileError("var : bad argument (argument 1 [name] must be a valid name)");
        }

        codeg::Keyword argPoolName;
        if ( !argPoolName.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check pool name
            throw codeg::CompileError("var : bad argument (argument 2 [name] must be a valid name)");
        }

        if ( codeg::Pool* tmpPool = data._pools.getPool(argPoolName._str) )
        {//Check pool
            if ( !tmpPool->addVariable( {argVarName._str, std::list<codeg::Address>()} ) )
            {
                codeg::ConsoleWrite("[warning] var : variable \""+argVarName._str+"\" already exist in pool \""+argPoolName._str+"\"");
            }
        }
        else
        {
            throw codeg::CompileError("var : bad pool (argument 2 \""+argPoolName._str+"\" the pool does not exist)");
        }

        return;
    }

    if ( input._keywords.size() == 2 )
    {//Check size
        codeg::Keyword argVarName;
        if ( !argVarName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check var name
            throw codeg::CompileError("var : bad argument (argument 1 [name] must be a valid name)");
        }

        if ( codeg::Pool* tmpPool = data._pools.getPool(data._defaultPool) )
        {//Check pool
            if ( !tmpPool->addVariable( {argVarName._str, std::list<codeg::Address>()} ) )
            {
                codeg::ConsoleWrite("[warning] var : variable \""+argVarName._str+"\" already exist in pool \""+data._defaultPool+"\"");
            }
        }
        else
        {
            throw codeg::FatalError("the default pool appear to not be created");
        }

        return;
    }

    throw codeg::CompileError("var : bad arguments size (wanted 2-3 got "+std::to_string(input._keywords.size())+")");
}

///Instruction_label
Instruction_label::Instruction_label(){}
Instruction_label::~Instruction_label(){}

std::string Instruction_label::getName() const
{
    return "label";
}

void Instruction_label::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("label : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {//Check label name
        throw codeg::CompileError("label : bad argument (argument 1 [name] must be a valid name)");
    }

    codeg::Label tmpLabel;
    tmpLabel._addressStatic = data._code._cursor;
    tmpLabel._uniqueIndex = 0;
    tmpLabel._name = argName._str;

    if ( !data._jumps.addLabel(tmpLabel) )
    {//Check name
        throw codeg::CompileError("label : bad label (label "+argName._str+" already exist)");
    }
}

///Instruction_jump
Instruction_jump::Instruction_jump(){}
Instruction_jump::~Instruction_jump(){}

std::string Instruction_jump::getName() const
{
    return "jump";
}

void Instruction_jump::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("jump : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {//Check label name
        throw codeg::CompileError("jump : bad argument (argument 1 [name] must be a valid name)");
    }

    codeg::JumpPoint tmpPoint;
    tmpPoint._addressStatic = data._code._cursor;
    tmpPoint._labelName = argName._str;

    if ( !data._jumps.addJumpPoint(tmpPoint) )
    {
        throw codeg::CompileError("jump : bad label (unknown label \""+argName._str+"\")");
    }

    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_restart
Instruction_restart::Instruction_restart(){}
Instruction_restart::~Instruction_restart(){}

std::string Instruction_restart::getName() const
{
    return "restart";
}

void Instruction_restart::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 1 )
    {//Check size
        throw codeg::CompileError("restart : bad arguments size (wanted 1 got "+std::to_string(input._keywords.size())+")");
    }

    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_affect
Instruction_affect::Instruction_affect(){}
Instruction_affect::~Instruction_affect(){}

std::string Instruction_affect::getName() const
{
    return "affect";
}

void Instruction_affect::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 3 )
    {//Check size
        throw codeg::CompileError("affect : bad arguments size (wanted 3 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argVar;
    if ( !argVar.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
    {//Check variable
        throw codeg::CompileError("affect : bad argument (argument 1 [variable] must be a valid variable)");
    }

    codeg::Keyword argValue;
    if ( argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValue._valueSize != 1)
        {
            throw codeg::CompileError("affect : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
        }

        argVar._variable->_link.push_back(data._code._cursor);

        data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);

        data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
        data._code.push(argValue._value);
        return;
    }
    else
    {//Possibly a variable
        if ( argValue._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {
            throw codeg::CompileError("affect : bad argument (argument 2 [value] can't be (for now) a variable)");
        }
        else
        {
            throw codeg::CompileError("affect : bad argument (argument 2 \""+argValue._str+"\" is not a value)");
        }
    }

    throw codeg::CompileError("affect : bad variable (unknown variable \""+argVar._str+"\")");
}

///Instruction_write
Instruction_write::Instruction_write(){}
Instruction_write::~Instruction_write(){}

std::string Instruction_write::getName() const
{
    return "write";
}

void Instruction_write::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 3 )
    {//Check size
        throw codeg::CompileError("write : bad arguments size (wanted 3 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argBus;
    if ( argBus.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_BUS, data) )
    {//A bus
        codeg::Keyword argValue;
        if ( argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if (argValue._valueSize != 1)
            {
                throw codeg::CompileError("write : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
            }
        }
        else
        {//Possibly a variable
            if ( argValue._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
            {
                argValue._variable->_link.push_back(data._code._cursor);

                data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
                data._code.push(0x00);
                data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
                data._code.push(0x00);
            }
            else
            {
                throw codeg::CompileError("write : bad argument (argument 2 \""+argValue._str+"\" is not a value)");
            }
        }

        switch ( argBus._value )
        {
        case codeg::BUS_NULL:
            throw codeg::CompileError("write : bad bus (bus can't be null)");
            break;

        case codeg::BUS_WRITEABLE_1:
            data._code.push(codeg::OPCODE_BWRITE1_CLK | argValue._valueBus);
            data._code.push(argValue._value);
            break;
        case codeg::BUS_WRITEABLE_2:
            data._code.push(codeg::OPCODE_BWRITE2_CLK | argValue._valueBus);
            data._code.push(argValue._value);
            break;
        case codeg::BUS_SPICFG:
            data._code.push(codeg::OPCODE_BCFG_SPI_CLK | argValue._valueBus);
            data._code.push(argValue._value);
            break;

        default:
            throw codeg::CompileError("write : bad bus (unknown bus)");
            break;
        }
    }
    else
    {
        throw codeg::CompileError("write : bad argument (argument 1 \""+argBus._str+"\" is not a bus");
    }
}

///Instruction_choose
Instruction_choose::Instruction_choose(){}
Instruction_choose::~Instruction_choose(){}

std::string Instruction_choose::getName() const
{
    return "choose";
}

void Instruction_choose::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 3 )
    {//Check size
        throw codeg::CompileError("choose : bad arguments size (wanted 3 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argTarget;
    if ( argTarget.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_TARGET, data) )
    {//A target
        codeg::Keyword argValue;
        if ( argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if (argValue._valueSize != 1)
            {
                throw codeg::CompileError("choose : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
            }
        }
        else
        {//Possibly a variable
            if ( argValue._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
            {
                argValue._variable->_link.push_back(data._code._cursor);

                data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
                data._code.push(0x00);
                data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
                data._code.push(0x00);
            }
            else
            {
                throw codeg::CompileError("choose : bad argument (argument 2 \""+argValue._str+"\" is not a value)");
            }
        }

        switch ( argTarget._target )
        {
        case codeg::TargetType::TARGET_OPERATION:
            data._code.push(codeg::OPCODE_OPCHOOSE_CLK | argValue._valueBus);
            data._code.push(argValue._value);
            break;
        case codeg::TargetType::TARGET_PERIPHERAL:
            data._code.push(codeg::OPCODE_BPCS_CLK | argValue._valueBus);
            data._code.push(argValue._value);
            break;

        default:
            throw codeg::CompileError("choose : bad target (unknown target)");
            break;
        }
    }
    else
    {
        throw codeg::CompileError("choose : bad argument (argument 1 \""+argTarget._str+"\" is not a target)");
    }
}

///Instruction_do
Instruction_do::Instruction_do(){}
Instruction_do::~Instruction_do(){}

std::string Instruction_do::getName() const
{
    return "do";
}

void Instruction_do::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 4 )
    {//Check size
        throw codeg::CompileError("do : bad arguments size (wanted 4 got "+std::to_string(input._keywords.size())+")");
    }

    ///LEFT
    codeg::Keyword argValueLeft;
    if ( argValueLeft.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValueLeft._valueSize != 1)
        {
            throw codeg::CompileError("do : bad value (require size is 1 byte got \""+std::to_string(argValueLeft._valueSize)+"\")");
        }
    }
    else
    {//Possibly a variable
        if ( argValueLeft._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {
            argValueLeft._variable->_link.push_back(data._code._cursor);

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
        }
        else
        {
            throw codeg::CompileError("do : bad argument (argument 1 \""+argValueLeft._str+"\" is not a value)");
        }
    }
    data._code.push(codeg::OPCODE_OPLEFT_CLK | argValueLeft._valueBus);
    data._code.push(argValueLeft._value);

    ///OPERATION
    codeg::Keyword argValueOp;
    if ( argValueOp.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValueOp._valueSize != 1)
        {
            throw codeg::CompileError("do : bad value (require size is 1 byte got \""+std::to_string(argValueOp._valueSize)+"\")");
        }
    }
    else
    {//Possibly a variable
        if ( argValueOp._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {
            argValueOp._variable->_link.push_back(data._code._cursor);

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
        }
        else
        {
            throw codeg::CompileError("do : bad argument (argument 2 \""+argValueOp._str+"\" is not a value)");
        }
    }
    data._code.push(codeg::OPCODE_OPCHOOSE_CLK | argValueOp._valueBus);
    data._code.push(argValueOp._value);

    ///RIGHT
    codeg::Keyword argValueRight;
    if ( argValueRight.process(input._keywords[3], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValueRight._valueSize != 1)
        {
            throw codeg::CompileError("do : bad value (require size is 1 byte got \""+std::to_string(argValueRight._valueSize)+"\")");
        }
    }
    else
    {//Possibly a variable
        if ( argValueRight._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {
            argValueRight._variable->_link.push_back(data._code._cursor);

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
        }
        else
        {
            throw codeg::CompileError("do : bad argument (argument 3 \""+argValueRight._str+"\" is not a value)");
        }
    }
    data._code.push(codeg::OPCODE_OPRIGHT_CLK | argValueRight._valueBus);
    data._code.push(argValueRight._value);
}

///Instruction_tick
Instruction_tick::Instruction_tick(){}
Instruction_tick::~Instruction_tick(){}

std::string Instruction_tick::getName() const
{
    return "tick";
}

void Instruction_tick::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._keywords.size() != 2) && (input._keywords.size() != 3) )
    {//Check size
        throw codeg::CompileError("tick : bad arguments size (wanted 2-3 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argStr;
    if ( argStr.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//A string
        if (argStr._str == "simple")
        {
            if ( input._keywords.size() == 3 )
            {//Check size
                codeg::Keyword argValue;
                if ( !argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
                {
                    if ( !argValue._valueConst )
                    {
                        throw codeg::CompileError("tick : bad argument (argument 2 [value] must be a valid constant value)");
                    }
                }
                for (uint32_t i=0; i<argValue._value; ++i)
                {
                    data._code.push(codeg::OPCODE_STICK);
                    data._code.push(0x00);
                }
            }
            else
            {
                data._code.push(codeg::OPCODE_STICK);
                data._code.push(0x00);
            }
        }
        else if (argStr._str == "long")
        {
            if ( input._keywords.size() == 3 )
            {//Check size
                codeg::Keyword argValue;
                if ( !argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
                {
                    if ( !argValue._valueConst )
                    {
                        throw codeg::CompileError("tick : bad argument (argument 2 [value] must be a valid constant value)");
                    }
                }
                for (uint32_t i=0; i<argValue._value; ++i)
                {
                    data._code.push(codeg::OPCODE_LTICK);
                    data._code.push(0x00);
                }
            }
            else
            {
                data._code.push(codeg::OPCODE_LTICK);
                data._code.push(0x00);
            }
        }
        else
        {
            throw codeg::CompileError("tick : bad argument (argument 1 [string] must be \"long\" or \"simple\")");
        }
    }
    else
    {
        throw codeg::CompileError("tick : bad string (argument 1 is not a string)");
    }
}

///Instruction_brut
Instruction_brut::Instruction_brut(){}
Instruction_brut::~Instruction_brut(){}

std::string Instruction_brut::getName() const
{
    return "brut";
}

void Instruction_brut::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() <= 1 )
    {//Check size
        throw codeg::CompileError("tick : bad arguments size (wanted >1 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argValue;

    for (uint32_t i=1; i<input._keywords.size(); ++i)
    {
        if ( argValue.process(input._keywords[i], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if ( argValue._valueConst )
            {
                if ( argValue._valueSize == 1 )
                {
                    data._code.push(argValue._value);
                }
                else
                {
                    throw codeg::CompileError("brut : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
                }
            }
            else
            {
                throw codeg::CompileError("brut : bad value (argument "+std::to_string(i)+" \""+argValue._str+"\" is not a valid constant value)");
            }
        }
        else
        {
            throw codeg::CompileError("brut : bad argument (argument "+std::to_string(i)+" [value] is not a value)");
        }
    }
}

///Instruction_function
Instruction_function::Instruction_function(){}
Instruction_function::~Instruction_function(){}

std::string Instruction_function::getName() const
{
    return "function";
}

void Instruction_function::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("function : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {
        throw codeg::CompileError("function : bad argument (argument 1 [name] bad name)");
    }

    if ( std::find(data._functions.begin(), data._functions.end(), argName._str) != data._functions.end() )
    {
        throw codeg::CompileError("function : bad function (function \""+argName._str+"\" already exist)");
    }

    if ( !data._jumps.addLabel({argName._str, 0, data._code._cursor}) )
    {
        throw codeg::CompileError("function : label error (label \""+argName._str+"\" already exist)");
    }

    if ( !data._actualFunctionName.empty() )
    {
        throw codeg::CompileError("function : function error (can't create a function in a function)");
    }
    if ( data._scope.size() )
    {
        throw codeg::CompileError("function : function error (can't create a function in a scope)");
    }

    data._actualFunctionName = argName._str;
    data._functions.push_back(argName._str);

    data._jumps._jumpPoints.push_back({"%%"+argName._str, data._code._cursor});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_if
Instruction_if::Instruction_if(){}
Instruction_if::~Instruction_if(){}

std::string Instruction_if::getName() const
{
    return "if";
}

void Instruction_if::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("if : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argValue;
    if ( argValue.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValue._valueSize != 1)
        {
            throw codeg::CompileError("if : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
        }
    }
    else
    {//Possibly a variable
        if ( argValue._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {
            argValue._variable->_link.push_back(data._code._cursor);

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
        }
        else
        {
            throw codeg::CompileError("if : bad argument (argument 1 \""+argValue._str+"\" is not a value)");
        }
    }
    /*
    2 jump points must be created in order to work properly :
        - At the false condition, label named %%Fn where n is the scope
        - At the end of the condition, label named %%En where n is the scope
    if there is no "else" keyword, the label %%Fn will be the end of the condition
    */

    data._scope.push(++data._scopeCount); //New scope
    data._scopeStats.push(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE);

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopeCount), data._code._cursor});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);


    data._code.push(codeg::OPCODE_IF | argValue._valueBus);
    data._code.push(argValue._value);

    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_else
Instruction_else::Instruction_else(){}
Instruction_else::~Instruction_else(){}

std::string Instruction_else::getName() const
{
    return "else";
}

void Instruction_else::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 1 )
    {//Check size
        throw codeg::CompileError("else : bad arguments size (wanted 1 got "+std::to_string(input._keywords.size())+")");
    }

    if ( data._scope.empty() )
    {
        throw codeg::CompileError("else : scope error (else must be placed in a valid conditional scope)");
    }

    if ( data._scopeStats.top() != codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE )
    {
        throw codeg::CompileError("else : scope error (else must be placed after a conditional keyword)");
    }

    data._jumps._jumpPoints.push_back({"%%E"+std::to_string(data._scopeCount), data._code._cursor});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);

    if ( !data._jumps.addLabel({"%%F"+std::to_string(data._scope.top()), 0, data._code._cursor}) )
    {
        throw codeg::CompileError("else : label error (label \"%%F"+std::to_string(data._scope.top())+"\" already exist)");
    }

    data._scopeStats.top() = codeg::ScopeStats::SCOPE_CONDITIONAL_FALSE;
}

///Instruction_ifnot
Instruction_ifnot::Instruction_ifnot(){}
Instruction_ifnot::~Instruction_ifnot(){}

std::string Instruction_ifnot::getName() const
{
    return "if_not";
}

void Instruction_ifnot::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("if_not : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argValue;
    if ( argValue.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValue._valueSize != 1)
        {
            throw codeg::CompileError("if_not : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
        }
    }
    else
    {//Possibly a variable
        if ( argValue._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {
            argValue._variable->_link.push_back(data._code._cursor);

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
        }
        else
        {
            throw codeg::CompileError("if_not : bad argument (argument 1 \""+argValue._str+"\" is not a value)");
        }
    }
    /*
    2 jump points must be created in order to work properly :
        - At the false condition, label named %%Fn where n is the scope
        - At the end of the condition, label named %%En where n is the scope
    if there is no "else" keyword, the label %%Fn will be the end of the condition
    */

    data._scope.push(++data._scopeCount); //New scope
    data._scopeStats.push(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE);

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopeCount), data._code._cursor});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);


    data._code.push(codeg::OPCODE_IFNOT | argValue._valueBus);
    data._code.push(argValue._value);

    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_end
Instruction_end::Instruction_end(){}
Instruction_end::~Instruction_end(){}

std::string Instruction_end::getName() const
{
    return "end";
}

void Instruction_end::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 1 )
    {//Check size
        throw codeg::CompileError("end : bad arguments size (wanted 1 got "+std::to_string(input._keywords.size())+")");
    }

    if ( data._scope.empty() )
    {
        if ( data._actualFunctionName.empty() )
        {
            throw codeg::CompileError("end : scope error (end must be placed to end a scope or a function)");
        }
        else
        {//End a function
            if ( !data._jumps.addLabel({"%%"+data._actualFunctionName, 0, data._code._cursor}) )
            {
                throw codeg::CompileError("end : label error (label \"%%"+data._actualFunctionName+"\" already exist)");
            }
            data._actualFunctionName.clear();
            return;
        }
    }
    //End a scope

    switch ( data._scopeStats.top() )
    {
    case codeg::ScopeStats::SCOPE_CONDITIONAL_FALSE:
        //Ending a conditional scope with the "else" keyword
        if ( !data._jumps.addLabel({"%%E"+std::to_string(data._scope.top()), 0, data._code._cursor}) )
        {
            throw codeg::CompileError("end : label error (label \"%%E"+std::to_string(data._scope.top())+"\" already exist)");
        }
        break;
    case codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE:
        //Ending a conditional scope without the "else" keyword
        if ( !data._jumps.addLabel({"%%F"+std::to_string(data._scope.top()), 0, data._code._cursor}) )
        {
            throw codeg::CompileError("end : label error (label \"%%F"+std::to_string(data._scope.top())+"\" already exist)");
        }
        if ( !data._jumps.addLabel({"%%E"+std::to_string(data._scope.top()), 0, data._code._cursor}) )
        {
            throw codeg::CompileError("end : label error (label \"%%E"+std::to_string(data._scope.top())+"\" already exist)");
        }
        break;
    default:
        throw codeg::FatalError("Bad scope stat !");
        break;
    }

    data._scope.pop();
    data._scopeStats.pop();
}

///Instruction_call
Instruction_call::Instruction_call(){}
Instruction_call::~Instruction_call(){}

std::string Instruction_call::getName() const
{
    return "call";
}

void Instruction_call::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("call : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {
        throw codeg::CompileError("call : bad argument (argument 1 \""+argName._str+"\" is not a name)");
    }

    if ( std::find(data._functions.begin(), data._functions.end(), argName._str) == data._functions.end() )
    {
        throw codeg::CompileError("call : bad function (unknown function \""+argName._str+"\")");
    }

    codeg::JumpPoint tmpPoint;
    tmpPoint._addressStatic = data._code._cursor;
    tmpPoint._labelName = argName._str;

    if ( !data._jumps.addJumpPoint(tmpPoint) )
    {
        throw codeg::CompileError("call : bad label (unknown label \""+argName._str+"\")");
    }

    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_clock
Instruction_clock::Instruction_clock(){}
Instruction_clock::~Instruction_clock(){}

std::string Instruction_clock::getName() const
{
    return "clock";
}

void Instruction_clock::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._keywords.size() != 1) && (input._keywords.size() != 2) )
    {//Check size
        throw codeg::CompileError("clock : bad arguments size (wanted 1-2 got "+std::to_string(input._keywords.size())+")");
    }

    if (input._keywords.size() == 1)
    {
        data._code.push(codeg::OPCODE_PERIPHERAL_CLK);
        data._code.push(0x00);
        return;
    }

    codeg::Keyword argValue;
    if ( argValue.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if ( argValue._valueConst )
        {
            for (uint32_t i=0; i<argValue._value; ++i)
            {
                data._code.push(codeg::OPCODE_PERIPHERAL_CLK);
                data._code.push(0x00);
            }
        }
        else
        {
            throw codeg::CompileError("clock : bad value (argument 1 \""+argValue._str+"\" is not a valid constant value)");
        }
    }
    else
    {
        throw codeg::CompileError("clock : bad argument (argument 1 [value] is not a value)");
    }
}

///Instruction_pool
Instruction_pool::Instruction_pool(){}
Instruction_pool::~Instruction_pool(){}

std::string Instruction_pool::getName() const
{
    return "pool";
}

void Instruction_pool::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._keywords.size() != 3) && (input._keywords.size() != 4) )
    {//Check size
        throw codeg::CompileError("pool : bad arguments size (wanted 3-4 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {//A name
        throw codeg::CompileError("pool : bad argument (argument 1 [name] is not a name)");
    }

    codeg::Keyword argSize;
    if ( !argSize.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        throw codeg::CompileError("pool : bad argument (argument 2 [value] is not a value)");
    }
    if ( !argSize._valueConst )
    {
        throw codeg::CompileError("pool : bad value (argument 2 [value] must be a valid constant value)");
    }

    codeg::Keyword argStart;
    argStart._value = 0;

    if (input._keywords.size() == 4)
    {
        if ( !argStart.process(input._keywords[3], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            throw codeg::CompileError("pool : bad argument (argument 3 [value] is not a value)");
        }
        if ( !argStart._valueConst )
        {
            throw codeg::CompileError("pool : bad value (argument 3 [value] must be a valid constant value)");
        }
    }

    if ( codeg::Pool* tmpPool = data._pools.getPool(argName._str) )
    {//Pool already exist
        codeg::ConsoleWrite("Warning : pool \""+argName._str+"\" already exist and will be replaced !");

        if (argStart._value == 0)
        {
            tmpPool->setStartAddressType(codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC);
            tmpPool->setAddress(argStart._value, argSize._value);
        }
        else
        {
            tmpPool->setStartAddressType(codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC);
            tmpPool->setAddress(argStart._value, argSize._value);
        }
    }
    else
    {//Pool must be created
        codeg::Pool tmpNewPool(argName._str);

        if (argStart._value == 0)
        {
            tmpNewPool.setStartAddressType(codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC);
            tmpNewPool.setAddress(argStart._value, argSize._value);
        }
        else
        {
            tmpNewPool.setStartAddressType(codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC);
            tmpNewPool.setAddress(argStart._value, argSize._value);
        }

        if ( !data._pools.addPool(tmpNewPool) )
        {
            throw codeg::CompileError("pool : bad pool (pool \""+argName._str+"\" already exist)");
        }
    }
}

}//end codeg
