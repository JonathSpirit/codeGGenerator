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
#include "C_error.hpp"

namespace codeg
{

const char* ReadableStringBinaryOpcodes[]=
{
    "BWRITE1_CLK",
    "BWRITE2_CLK",

    "BPCS_CLK",

    "OPLEFT_CLK",
    "OPRIGHT_CLK",
    "OPCHOOSE_CLK",

    "PERIPHERAL_CLK",

    "BJMPSRC1_CLK",
    "BJMPSRC2_CLK",
    "BJMPSRC3_CLK",
    "JMPSRC_CLK",

    "BRAMADD1_CLK",
    "BRAMADD2_CLK",

    "SPI_CLK",
    "BCFG_SPI_CLK",

    "STICK",

    "IF",
    "IFNOT",

    "RAMW",

    "UOP",
    "UOP",
    "UOP",
    "UOP",

    "LTICK"
};

///Instruction

void Instruction::compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    data._functions.getLast()->addLine(input._cleaned);
}

///InstructionList

void InstructionList::clear()
{
    this->g_data.clear();
}

void InstructionList::push(codeg::Instruction* newInstruction)
{
    this->g_data.push_front( std::unique_ptr<codeg::Instruction>(newInstruction) );
}
codeg::Instruction* InstructionList::get(const std::string& name) const
{
    for (auto& valPtr : this->g_data)
    {
        if (valPtr.get()->getName() == name)
        {
            return valPtr.get();
        }
    }
    return nullptr;
}

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
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check name
        throw codeg::CompileError("set : bad argument (argument 1 [string] must be a valid string)");
    }

    codeg::Keyword argString;
    if ( !argString.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check string
        throw codeg::CompileError("set : bad argument (argument 2 [string] must be a valid string)");
    }

    if ( data._macros.check(argName._str) )
    {//Check if already set
        data._macros.set(argName._str, argString._str);
        codeg::ConsoleWarningWrite("[warning] set : macro \""+ argName._str +"\" already exist and will be replaced");
    }
    else
    {
        data._macros.set(argName._str, argString._str);
    }
}

///Instruction_unset
Instruction_unset::Instruction_unset(){}
Instruction_unset::~Instruction_unset(){}

std::string Instruction_unset::getName() const
{
    return "unset";
}

void Instruction_unset::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("unset : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check name
        throw codeg::CompileError("unset : bad argument (argument 1 [string] must be a valid string)");
    }

    data._macros.remove(argName._str);
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
    if ( input._keywords.size() == 2 )
    {//Label name with determined cursor address
        codeg::Keyword argName;
        if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check label name
            throw codeg::CompileError("label : bad argument (argument 1 [name] must be a valid name)");
        }

        codeg::Label tmpLabel;
        tmpLabel._addressStatic = data._code.getCursor();
        tmpLabel._uniqueIndex = 0;
        tmpLabel._name = argName._str;

        if ( !data._jumps.addLabel(tmpLabel) )
        {//Check name
            throw codeg::CompileError("label : bad label (label "+argName._str+" already exist)");
        }
    }
    else if ( input._keywords.size() == 3 )
    {//Label name with fixed address
        codeg::Keyword argName;
        if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check label name
            throw codeg::CompileError("label : bad argument (argument 1 [name] must be a valid name)");
        }
        codeg::Keyword argValue;
        if ( !argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//Check const value
            throw codeg::CompileError("label : bad argument (argument 2 [value] must be a valid constant value)");
        }
        if ( !argValue._valueIsConst )
        {
            throw codeg::CompileError("label : bad argument (argument 2 [value] must be a valid constant value)");
        }

        codeg::Label tmpLabel;
        tmpLabel._addressStatic = argValue._value;
        tmpLabel._uniqueIndex = 0;
        tmpLabel._name = argName._str;

        if ( !data._jumps.addLabel(tmpLabel) )
        {//Check name
            throw codeg::CompileError("label : bad label (label "+argName._str+" already exist)");
        }
    }
    else
    {
        throw codeg::CompileError("label : bad arguments size (wanted 2 or 3 got "+std::to_string(input._keywords.size())+")");
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
    if ( input._keywords.size() == 2 )
    {//Label name or fixed address
        codeg::Keyword arg1;
        if ( arg1.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check label name
            codeg::JumpPoint tmpPoint;
            tmpPoint._addressStatic = data._code.getCursor();
            tmpPoint._labelName = arg1._str;

            if ( !data._jumps.addJumpPoint(tmpPoint) )
            {
                throw codeg::CompileError("jump : bad label (unknown label \""+arg1._str+"\")");
            }

            data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_JMPSRC_CLK);
        }
        else if ( arg1._type == codeg::KeywordTypes::KEYWORD_VALUE )
        {//Check constant value
            if ( arg1._valueSize <= 3 )
            {
                uint32_t address = arg1._value;

                data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
                data._code.push( (address&0x00FF0000)>>16 );
                data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
                data._code.push( (address&0x0000FF00)>>8 );
                data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
                data._code.push( address&0x000000FF );
                data._code.push(codeg::OPCODE_JMPSRC_CLK);
            }
            else
            {
                throw codeg::CompileError("jump : bad size (argument 1 [value] byte size must be <= 3)");
            }
        }
        else
        {
            throw codeg::CompileError("jump : bad argument (argument 1 [name]/[value] must be a valid name or constant value)");
        }
    }
    else if ( input._keywords.size() == 4 )
    {//3 separate values or variables
        codeg::Keyword arg1;
        if ( arg1.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A constant value
            if ( arg1._valueSize == 1 )
            {
                data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
                data._code.push( arg1._value );
            }
            else
            {
                throw codeg::CompileError("jump : bad size (argument 1 [value] byte size must be == 1)");
            }
        }
        else if ( arg1._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {//A variable
            arg1._variable->_link.push_back(data._code.getCursor());

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_RAM);
            data._code.pushDummy();
        }
        else
        {
            throw codeg::CompileError("jump : bad argument (argument 1 [value] must be a valid value)");
        }

        codeg::Keyword arg2;
        if ( arg2.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A constant value
            if ( arg2._valueSize == 1 )
            {
                data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
                data._code.push( arg2._value );
            }
            else
            {
                throw codeg::CompileError("jump : bad size (argument 2 [value] byte size must be == 1)");
            }
        }
        else if ( arg2._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {//A variable
            arg2._variable->_link.push_back(data._code.getCursor());

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_RAM);
            data._code.pushDummy();
        }
        else
        {
            throw codeg::CompileError("jump : bad argument (argument 2 [value] must be a valid value)");
        }

        codeg::Keyword arg3;
        if ( arg3.process(input._keywords[3], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A constant value
            if ( arg3._valueSize == 1 )
            {
                data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
                data._code.push( arg3._value );
            }
            else
            {
                throw codeg::CompileError("jump : bad size (argument 3 [value] byte size must be == 1)");
            }
        }
        else if ( arg3._type == codeg::KeywordTypes::KEYWORD_VARIABLE )
        {//A variable
            arg3._variable->_link.push_back(data._code.getCursor());

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_RAM);
            data._code.pushDummy();
        }
        else
        {
            throw codeg::CompileError("jump : bad argument (argument 3 [value] must be a valid value)");
        }

        data._code.push(codeg::OPCODE_JMPSRC_CLK);
    }
    else
    {//Bad size
        throw codeg::CompileError("jump : bad arguments size (wanted 2 or 4 got "+std::to_string(input._keywords.size())+")");
    }
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
    if ( input._keywords.size() == 3 )
    {//fixed specified address or variable
        codeg::Keyword argVar;
        if ( argVar.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {//Variable
            codeg::Keyword argValue;
            if ( argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
            {//A value
                if ( !argValue._valueIsVariable )
                {
                    if (argValue._valueSize != 1)
                    {
                        throw codeg::CompileError("affect : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
                    }

                    argVar._variable->_link.push_back(data._code.getCursor());

                    data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
                    data._code.push(0x00);
                    data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
                    data._code.push(0x00);

                    data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                    if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
                    {
                        data._code.push(argValue._value);
                    }
                    else
                    {
                        data._code.pushDummy();
                    }
                }
                else
                {//A variable
                    throw codeg::CompileError("affect : can't copy (for now) a variable in another variable");
                }
            }
            else
            {
                throw codeg::CompileError("affect : bad argument (argument 2 \""+argValue._str+"\" is not a value)");
            }
        }
        else if ( argVar._type == codeg::KeywordTypes::KEYWORD_CONSTANT )
        {//Constant
            if ( argVar._valueSize > 2 )
            {
                throw codeg::CompileError("affect : bad constant (require size is <= 2 byte got \""+std::to_string(argVar._valueSize)+"\")");
            }

            codeg::Keyword argValue;
            if ( argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
            {//A value
                if ( !argValue._valueIsVariable )
                {
                    if (argValue._valueSize != 1)
                    {
                        throw codeg::CompileError("affect : bad value (require size is 1 byte got \""+std::to_string(argValue._valueSize)+"\")");
                    }

                    data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
                    data._code.push((argVar._value&0xFF00)>>8);
                    data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
                    data._code.push(argVar._value&0x00FF);

                    data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                    if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
                    {
                        data._code.push(argValue._value);
                    }
                    else
                    {
                        data._code.pushDummy();
                    }
                }
                else
                {//A variable
                    throw codeg::CompileError("affect : can't copy (for now) a variable in another variable");
                }
            }
            else
            {
                throw codeg::CompileError("affect : bad argument (argument 2 \""+argValue._str+"\" is not a value)");
            }
        }
        else
        {
            throw codeg::CompileError("affect : bad argument (argument 1 [variable]/[constant] must be valid)");
        }
    }
    else if ( input._keywords.size() >= 4 )
    {//fixed size pool
        codeg::Keyword argPoolName;
        if ( argPoolName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Pool name
            codeg::Pool* pool = data._pools.getPool(argPoolName._str);
            if (!pool)
            {
                throw codeg::CompileError("affect : bad argument (unknown pool : \""+argPoolName._str+"\")");
            }
            if ( pool->getMaxSize() == 0 )
            {
                throw codeg::CompileError("affect : bad argument (pool must have a fixed size)");
            }

            codeg::Keyword argOffset;
            if ( argOffset.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
            {//Offset
                if (argOffset._valueSize > 2)
                {
                    throw codeg::CompileError("affect : bad argument (argument 2 [constant] must have a byte size of <= 2)");
                }

                codeg::Address offset = argOffset._value;
                unsigned int numOfValue = input._keywords.size() - 3;
                if ( (numOfValue+offset) > pool->getMaxSize())
                {
                    throw codeg::CompileError("affect : pool overflow (try to affect "+std::to_string(numOfValue)+" values with offset "+std::to_string(offset)+" but the max size is "+std::to_string(pool->getMaxSize())+")");
                }

                for (unsigned int i=0; i<numOfValue; ++i)
                {
                    codeg::Keyword argValue;
                    if ( argValue.process(input._keywords[i+3], codeg::KeywordTypes::KEYWORD_VALUE, data) )
                    {//Value
                        if (argValue._valueSize != 1)
                        {
                            throw codeg::CompileError("affect : bad argument (argument "+std::to_string(i+3)+" [value] must have a byte size of 1)");
                        }

                        pool->_link.push_back({data._code.getCursor(), i+offset});

                        data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
                        data._code.push(0x00);
                        data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
                        data._code.push(0x00);

                        data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                        if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
                        {
                            data._code.push(argValue._value);
                        }
                        else
                        {
                            data._code.pushDummy();
                        }
                    }
                    else if ( argValue._valueIsVariable )
                    {
                        throw codeg::CompileError("affect : can't copy a variable in another location in memory");
                    }
                    else
                    {
                        throw codeg::CompileError("affect : bad argument (argument "+std::to_string(i+3)+" [value] must be a valid value)");
                    }
                }
            }
            else
            {
                throw codeg::CompileError("affect : bad argument (argument 2 [constant] must be a valid constant)");
            }
        }
        else
        {
            throw codeg::CompileError("affect : bad argument (argument 1 [name] must be a valid name)");
        }
    }
    else
    {
        throw codeg::CompileError("affect : bad arguments size (wanted 3 or 4 got "+std::to_string(input._keywords.size())+")");
    }
}

///Instruction_get
Instruction_get::Instruction_get(){}
Instruction_get::~Instruction_get(){}

std::string Instruction_get::getName() const
{
    return "get";
}

void Instruction_get::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() == 2 )
    {//fixed specified address or variable
        codeg::Keyword argVar;
        if ( argVar.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {//Variable
            argVar._variable->_link.push_back(data._code.getCursor());

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(0x00);
        }
        else if ( argVar._type == codeg::KeywordTypes::KEYWORD_CONSTANT )
        {//Constant
            if ( argVar._valueSize > 2 )
            {
                throw codeg::CompileError("get : bad constant (require size is <= 2 byte got \""+std::to_string(argVar._valueSize)+"\")");
            }

            data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
            data._code.push((argVar._value&0xFF00)>>8);
            data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
            data._code.push(argVar._value&0x00FF);
        }
        else
        {
            throw codeg::CompileError("get : bad argument (argument 1 [variable]/[constant] must be valid)");
        }
    }
    else if ( input._keywords.size() == 3 )
    {//fixed size pool
        codeg::Keyword argPoolName;
        if ( argPoolName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Pool name
            codeg::Pool* pool = data._pools.getPool(argPoolName._str);
            if (!pool)
            {
                throw codeg::CompileError("get : bad argument (unknown pool : \""+argPoolName._str+"\")");
            }
            if ( pool->getMaxSize() == 0 )
            {
                throw codeg::CompileError("get : bad argument (pool must have a fixed size)");
            }

            codeg::Keyword argOffset;
            if ( argOffset.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
            {//Offset
                if (argOffset._valueSize > 2)
                {
                    throw codeg::CompileError("get : bad argument (argument 2 [constant] must have a byte size of <= 2)");
                }

                codeg::Address offset = argOffset._value;
                if (offset >= pool->getMaxSize())
                {
                    throw codeg::CompileError("get : pool overflow (try to get value at offset "+std::to_string(offset)+" but the max size is "+std::to_string(pool->getMaxSize())+")");
                }

                pool->_link.push_back({data._code.getCursor(), offset});

                data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
                data._code.push(0x00);
                data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
                data._code.push(0x00);
            }
            else
            {
                throw codeg::CompileError("get : bad argument (argument 2 [constant] must be a valid constant)");
            }
        }
        else
        {
            throw codeg::CompileError("get : bad argument (argument 1 [name] must be a valid name)");
        }
    }
    else
    {
        throw codeg::CompileError("get : bad arguments size (wanted 2 or 3 got "+std::to_string(input._keywords.size())+")");
    }
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
                argValue._variable->_link.push_back(data._code.getCursor());

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
            break;
        case codeg::BUS_WRITEABLE_2:
            data._code.push(codeg::OPCODE_BWRITE2_CLK | argValue._valueBus);
            break;
        case codeg::BUS_SPICFG:
            data._code.push(codeg::OPCODE_BCFG_SPI_CLK | argValue._valueBus);
            break;

        default:
            throw codeg::CompileError("write : bad bus (unknown bus)");
            break;
        }

        if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
        {
            data._code.push(argValue._value);
        }
        else
        {
            data._code.pushDummy();
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
                argValue._variable->_link.push_back(data._code.getCursor());

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
            break;
        case codeg::TargetType::TARGET_PERIPHERAL:
            data._code.push(codeg::OPCODE_BPCS_CLK | argValue._valueBus);
            break;

        default:
            throw codeg::CompileError("choose : bad target (unknown target)");
            break;
        }

        if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
        {
            data._code.push(argValue._value);
        }
        else
        {
            data._code.pushDummy();
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
            argValueLeft._variable->_link.push_back(data._code.getCursor());

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
    if (argValueLeft._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
    {
        data._code.push(argValueLeft._value);
    }
    else
    {
        data._code.pushDummy();
    }

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
            argValueOp._variable->_link.push_back(data._code.getCursor());

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
    if (argValueOp._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
    {
        data._code.push(argValueOp._value);
    }
    else
    {
        data._code.pushDummy();
    }

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
            argValueRight._variable->_link.push_back(data._code.getCursor());

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
    if (argValueRight._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
    {
        data._code.push(argValueRight._value);
    }
    else
    {
        data._code.pushDummy();
    }
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
                    if ( !argValue._valueIsConst )
                    {
                        throw codeg::CompileError("tick : bad argument (argument 2 [value] must be a valid constant value)");
                    }
                }
                for (uint32_t i=0; i<argValue._value; ++i)
                {
                    data._code.push(codeg::OPCODE_STICK | codeg::READABLE_DEFAULT);
                    data._code.pushDummy();
                }
            }
            else
            {
                data._code.push(codeg::OPCODE_STICK | codeg::READABLE_DEFAULT);
                data._code.pushDummy();
            }
        }
        else if (argStr._str == "long")
        {
            if ( input._keywords.size() == 3 )
            {//Check size
                codeg::Keyword argValue;
                if ( !argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
                {
                    if ( !argValue._valueIsConst )
                    {
                        throw codeg::CompileError("tick : bad argument (argument 2 [value] must be a valid constant value)");
                    }
                }
                for (uint32_t i=0; i<argValue._value; ++i)
                {
                    data._code.push(codeg::OPCODE_LTICK | codeg::READABLE_DEFAULT);
                    data._code.pushDummy();
                }
            }
            else
            {
                data._code.push(codeg::OPCODE_LTICK | codeg::READABLE_DEFAULT);
                data._code.pushDummy();
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
            if ( argValue._valueIsConst )
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

    if ( data._functions.get(argName._str) != nullptr )
    {
        throw codeg::CompileError("function : bad function (function \""+argName._str+"\" already exist)");
    }

    if ( !data._actualFunctionName.empty() )
    {
        throw codeg::CompileError("function : function error (can't create a function in a function)");
    }
    if ( data._scopes.size() )
    {
        throw codeg::CompileError("function : function error (can't create a function in a scope)");
    }

    data._scopes.newScope(codeg::ScopeStats::SCOPE_FUNCTION, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._actualFunctionName = argName._str;
    data._functions.push(argName._str);

    data._jumps._jumpPoints.push_back({"%%E"+argName._str, data._code.getCursor()}); //Jump to the end of the function
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);

    if ( !data._jumps.addLabel({"%%"+argName._str, 0, data._code.getCursor()}) )
    {//Label to the start of the function
        throw codeg::CompileError("function : label error (label \"%%"+argName._str+"\" already exist)");
    }
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
            argValue._variable->_link.push_back(data._code.getCursor());

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

    data._scopes.newScope(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopes.getScopeCount()), data._code.getCursor()});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);


    data._code.push(codeg::OPCODE_IF | argValue._valueBus);
    if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
    {
        data._code.push(argValue._value);
    }
    else
    {
        data._code.pushDummy();
    }

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

    if ( data._scopes.empty() )
    {
        throw codeg::CompileError("else : scope error (else must be placed in a valid conditional scope)");
    }

    if ( data._scopes.top()._stat != codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE )
    {
        throw codeg::CompileError("else : scope error (else must be placed after a conditional keyword)");
    }

    data._jumps._jumpPoints.push_back({"%%E"+std::to_string(data._scopes.top()._id), data._code.getCursor()});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);

    if ( !data._jumps.addLabel({"%%F"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
    {
        throw codeg::CompileError("else : label error (label \"%%F"+std::to_string(data._scopes.top()._id)+"\" already exist)");
    }

    data._scopes.top()._stat = codeg::ScopeStats::SCOPE_CONDITIONAL_FALSE;
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
            argValue._variable->_link.push_back(data._code.getCursor());

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

    data._scopes.newScope(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopes.getScopeCount()), data._code.getCursor()});
    data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);
    data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
    data._code.push(0x00);


    data._code.push(codeg::OPCODE_IFNOT | argValue._valueBus);
    if (argValue._valueBus == codeg::ReadableBusses::READABLE_SOURCE)
    {
        data._code.push(argValue._value);
    }
    else
    {
        data._code.pushDummy();
    }

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

    if ( data._scopes.empty() )
    {
        throw codeg::CompileError("end : scope error ('end' must be placed to end a scope)");
    }

    //Ending a scope
    switch ( data._scopes.top()._stat )
    {
    case codeg::ScopeStats::SCOPE_FUNCTION:
        //Ending a function
        if ( !data._jumps.addLabel({"%%E"+data._actualFunctionName, 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("end : label error (label \"%%E"+data._actualFunctionName+"\" already exist)");
        }
        data._actualFunctionName.clear();
        break;
    case codeg::ScopeStats::SCOPE_CONDITIONAL_FALSE:
        //Ending a conditional scope with the "else" keyword
        if ( !data._jumps.addLabel({"%%E"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("end : label error (label \"%%E"+std::to_string(data._scopes.top()._id)+"\" already exist)");
        }
        break;
    case codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE:
        //Ending a conditional scope without the "else" keyword
        if ( !data._jumps.addLabel({"%%F"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("end : label error (label \"%%F"+std::to_string(data._scopes.top()._id)+"\" already exist)");
        }
        if ( !data._jumps.addLabel({"%%E"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("end : label error (label \"%%E"+std::to_string(data._scopes.top()._id)+"\" already exist)");
        }
        break;
    default:
        throw codeg::FatalError("Bad scope stat !");
        break;
    }

    data._scopes.pop();
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
    if ( input._keywords.size() == 5 )
    {//call a function
        codeg::Keyword argName;
        if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {
            throw codeg::CompileError("call : bad argument (argument 1 \""+argName._str+"\" is not a name)");
        }
        codeg::Function* func = data._functions.get(argName._str);
        if ( func == nullptr )
        {
            throw codeg::CompileError("call : bad function (unknown function \""+argName._str+"\")");
        }
        if ( func->isDefinition() )
        {
            throw codeg::CompileError("call : bad function (can't call a definition with return address)");
        }

        codeg::Keyword argVar1;
        if ( !argVar1.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {
            throw codeg::CompileError("call : bad argument (argument 2 \""+argVar1._str+"\" is not a valid variable)");
        }
        codeg::Keyword argVar2;
        if ( !argVar2.process(input._keywords[3], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {
            throw codeg::CompileError("call : bad argument (argument 3 \""+argVar2._str+"\" is not a valid variable)");
        }
        codeg::Keyword argVar3;
        if ( !argVar3.process(input._keywords[4], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {
            throw codeg::CompileError("call : bad argument (argument 4 \""+argVar3._str+"\" is not a valid variable)");
        }

        //Prepare return address
        uint32_t returnAddress = data._code.getCursor() + 25;

        argVar1._variable->_link.push_back(data._code.getCursor()); //MSB
        data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push((returnAddress&0x00FF0000)>>16);

        argVar2._variable->_link.push_back(data._code.getCursor()); //MSB
        data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push((returnAddress&0x0000FF00)>>8);

        argVar3._variable->_link.push_back(data._code.getCursor()); //MSB
        data._code.push(codeg::OPCODE_BRAMADD2_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_BRAMADD1_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push(returnAddress&0x000000FF);

        codeg::JumpPoint tmpPoint;
        tmpPoint._addressStatic = data._code.getCursor();
        tmpPoint._labelName = "%%"+argName._str;

        if ( !data._jumps.addJumpPoint(tmpPoint) )
        {
            throw codeg::CompileError("call : bad label (unknown label \"%%"+argName._str+"\")");
        }
        data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_SOURCE);
        data._code.push(0x00);
        data._code.push(codeg::OPCODE_JMPSRC_CLK);
    }
    else if ( input._keywords.size() == 2 )
    {//call a definition
        codeg::Keyword argName;
        if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {
            throw codeg::CompileError("call : bad argument (argument 1 \""+argName._str+"\" is not a name)");
        }
        codeg::Function* func = data._functions.get(argName._str);
        if ( func == nullptr )
        {
            throw codeg::CompileError("call : bad definition (unknown definition \""+argName._str+"\")");
        }
        if ( !func->isDefinition() )
        {
            throw codeg::CompileError("call : bad definition (\""+argName._str+"\" is not a definition)");
        }

        data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_definition(func)) );
    }
    else
    {
        throw codeg::CompileError("call : bad arguments size (wanted 5 or 2 got "+std::to_string(input._keywords.size())+")");
    }
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
    if ( (input._keywords.size() != 2) && (input._keywords.size() != 3) )
    {//Check size
        throw codeg::CompileError("clock : bad arguments size (wanted 2-3 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argTarget;
    if ( !argTarget.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_TARGET, data) )
    {
        throw codeg::CompileError("clock : bad argument (argument 1 [target] is not a valid target)");
    }

    uint8_t targetOpcode;
    switch (argTarget._target)
    {
    case codeg::TARGET_PERIPHERAL:
        targetOpcode = codeg::OPCODE_PERIPHERAL_CLK;
        break;
    case codeg::TARGET_SPI:
        targetOpcode = codeg::OPCODE_SPI_CLK;
        break;
    default:
        throw codeg::CompileError("clock : bad argument (argument 1 [target] is not a valid target)");
        break;
    }

    if (input._keywords.size() == 3)
    {
        codeg::Keyword argValue;
        if ( argValue.process(input._keywords[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            for (uint32_t i=0; i<argValue._value; ++i)
            {
                data._code.push(targetOpcode | codeg::READABLE_DEFAULT);
                data._code.pushDummy();
            }
        }
        else
        {
            throw codeg::CompileError("clock : bad argument (argument 2 [value] is not valid constant value)");
        }
    }
    else
    {
        data._code.push(targetOpcode | codeg::READABLE_DEFAULT);
        data._code.pushDummy();
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
    if ( !argSize._valueIsConst )
    {
        throw codeg::CompileError("pool : bad value (argument 2 [value] must be a valid constant value)");
    }

    codeg::Keyword argStart;
    bool isDynamic = true;
    if (input._keywords.size() == 4)
    {
        if ( !argStart.process(input._keywords[3], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            throw codeg::CompileError("pool : bad argument (argument 3 [value] is not a value)");
        }
        if ( !argStart._valueIsConst )
        {
            throw codeg::CompileError("pool : bad value (argument 3 [value] must be a valid constant value)");
        }
        isDynamic = false;
    }

    if ( codeg::Pool* tmpPool = data._pools.getPool(argName._str) )
    {//Pool already exist
        codeg::ConsoleWarningWrite("Warning : pool \""+argName._str+"\" already exist and will be replaced !");

        if (isDynamic)
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

        if (isDynamic)
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

///Instruction_import
Instruction_import::Instruction_import(){}
Instruction_import::~Instruction_import(){}

std::string Instruction_import::getName() const
{
    return "import";
}

void Instruction_import::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("import : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    std::string path = data._relativePath + input._keywords[1];

    if ( !data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_file(path)) ) )
    {
        throw codeg::CompileError("import : can't open the file : "+path+")");
    }
}

///Instruction_definition
Instruction_definition::Instruction_definition(){}
Instruction_definition::~Instruction_definition(){}

std::string Instruction_definition::getName() const
{
    return "definition";
}

void Instruction_definition::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 2 )
    {//Check size
        throw codeg::CompileError("definition : bad arguments size (wanted 2 got "+std::to_string(input._keywords.size())+")");
    }

    codeg::Keyword argName;
    if ( !argName.process(input._keywords[1], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {
        throw codeg::CompileError("definition : bad argument (argument 1 [name] bad name)");
    }

    if ( data._functions.get(argName._str) != nullptr )
    {
        throw codeg::CompileError("definition : bad definition (definition/function \""+argName._str+"\" already exist)");
    }

    if ( data._scopes.size() )
    {
        throw codeg::CompileError("definition : definition error (can't create a definition in a scope)");
    }

    data._scopes.newScope(codeg::ScopeStats::SCOPE_DEFINITION, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._actualFunctionName = argName._str;
    data._functions.push(argName._str, true);

    data._writeLinesIntoDefinition = true;
}
///Instruction_enddef
Instruction_enddef::Instruction_enddef(){}
Instruction_enddef::~Instruction_enddef(){}

std::string Instruction_enddef::getName() const
{
    return "end_def";
}

void Instruction_enddef::compile( [[maybe_unused]] const codeg::StringDecomposer& input, [[maybe_unused]] codeg::CompilerData& data)
{
    throw codeg::CompileError("end_def : 'end_def' can only be placed to end a definition scope");
}
void Instruction_enddef::compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._keywords.size() != 1 )
    {//Check size
        throw codeg::CompileError("end_def : bad arguments size (wanted 1 got "+std::to_string(input._keywords.size())+")");
    }

    //Ending the definition
    data._writeLinesIntoDefinition = false;
    data._actualFunctionName.clear();

    data._scopes.pop();
}

}//end codeg
