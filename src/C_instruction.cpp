/////////////////////////////////////////////////////////////////////////////////
// Copyright 2022 Guillaume Guillet                                            //
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
#include <limits>

#define CODEG_INSTRUCTIONLIST_RESERVE_SIZE (0xFF&CODEG_BINARYOPCODES_MASK)

namespace codeg
{

namespace
{
const char* __stringBinaryOpcodes[]=
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
}//end

const char* OpcodeToString(uint8_t opcode)
{
    opcode = (opcode&CODEG_BINARYOPCODES_MASK);
    if ( (opcode&CODEG_BINARYOPCODES_MASK) > 0x17 )
    {
        return __stringBinaryOpcodes[0x13];
    }
    return __stringBinaryOpcodes[opcode&CODEG_BINARYOPCODES_MASK];
}

///Instruction

void Instruction::compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    data._functions.getLast()->addLine(input._cleaned);
}

///InstructionList

InstructionList::InstructionList()
{
    this->g_data.reserve(CODEG_INSTRUCTIONLIST_RESERVE_SIZE);
}

void InstructionList::clear()
{
    this->g_data.clear();
}

void InstructionList::push(std::unique_ptr<codeg::Instruction>&& newInstruction)
{
    this->g_data.push_back( std::move(newInstruction) );
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
std::string Instruction_set::getName() const
{
    return "set";
}

void Instruction_set::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 2 )
    {//Check size
        throw codeg::ArgumentsSizeError("2", input._arguments.size());
    }

    codeg::Keyword argStringName;
    if ( !argStringName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check string
        throw codeg::ArgumentError(1, "string");
    }

    codeg::Keyword argStringData;
    if ( !argStringData.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check string
        throw codeg::ArgumentError(2, "string");
    }

    if ( data._macros.check(argStringName._str) )
    {//Check if already set
        data._macros.set(argStringName._str, argStringData._str);
        codeg::ConsoleWarningWrite("macro \""+ argStringName._str +"\" already exist and will be replaced");
    }
    else
    {
        data._macros.set(argStringName._str, argStringData._str);
    }
}

///Instruction_unset
std::string Instruction_unset::getName() const
{
    return "unset";
}

void Instruction_unset::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 1 )
    {//Check size
        throw codeg::ArgumentsSizeError("1", input._arguments.size());
    }

    codeg::Keyword argStringName;
    if ( !argStringName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//Check string
        throw codeg::ArgumentError(1, "string");
    }

    data._macros.remove(argStringName._str);
}

///Instruction_var
std::string Instruction_var::getName() const
{
    return "var";
}

void Instruction_var::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._arguments.size() != 1) && (input._arguments.size() != 2)  )
    {
        throw codeg::ArgumentsSizeError("1,2", input._arguments.size());
    }

    std::string varName;
    std::string varPool;
    if ( !codeg::GetVariableString(input._arguments[0], data._defaultPool, varName, varPool) )
    {//Check variable
        throw codeg::ArgumentError(1, "variable");
    }

    codeg::MemorySize varSize = 1;
    if (input._arguments.size() == 2)
    {
        codeg::Keyword argConstant;
        if ( !argConstant.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
        {//Check constant
            throw codeg::ArgumentError(2, "constant");
        }
        if ( (argConstant._value == 0) || (argConstant._value > std::numeric_limits<codeg::MemorySize>::max()) )
        {
            throw codeg::CompileError("variable size cannot be 0 or >"+std::to_string(std::numeric_limits<codeg::MemorySize>::max()));
        }
        varSize = argConstant._value;
    }

    if ( codeg::Pool* tmpPool = data._pools.getPool(varPool) )
    {//Check pool
        if ( codeg::Variable* buffVar = tmpPool->getVariable(varName) )
        {
            if (buffVar->_size == varSize)
            {
                codeg::ConsoleWarningWrite("redeclaration of variable \""+varName+"\" in pool \""+varPool+"\" with size of "+std::to_string(varSize));
            }
            else
            {
                throw codeg::CompileError("redeclaration of variable \""+varName+"\" in pool \""+varPool+"\" but with different size (wanted "+std::to_string(varSize)+" instead of "+std::to_string(buffVar->_size)+")");
            }
        }
        else
        {
            if ( !tmpPool->addVariable(varName, varSize) )
            {
                throw codeg::FatalError("Unknown error, variable should be ok but can't create it");
            }
        }
    }
    else
    {
        throw codeg::CompileError("bad pool (pool \""+varPool+"\" does not exist)");
    }
}

///Instruction_label
std::string Instruction_label::getName() const
{
    return "label";
}

void Instruction_label::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    uint32_t address;

    if ( input._arguments.size() == 2)
    {//Custom address position
        codeg::Keyword argConstant;
        if ( !argConstant.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
        {//Check constant
            throw codeg::ArgumentError(2, "constant");
        }

        address = argConstant._value;
    }
    else if ( input._arguments.size() == 1)
    {
        address = data._code.getCursor();
    }
    else
    {
        throw codeg::ArgumentsSizeError("1-2", input._arguments.size());
    }

    codeg::Keyword argName;
    if ( !argName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {//Check label name
        throw codeg::ArgumentError(1, "name");
    }

    codeg::Label tmpLabel;
    tmpLabel._addressStatic = address;
    tmpLabel._uniqueIndex = 0;
    tmpLabel._name = argName._str;

    if ( !data._jumps.addLabel(tmpLabel) )
    {//Check label
        throw codeg::CompileError("bad label (label \""+argName._str+"\" already exist)");
    }
}

///Instruction_jump
std::string Instruction_jump::getName() const
{
    return "jump";
}

void Instruction_jump::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() == 1)
    {//Label or fixed address
        codeg::Keyword arg;
        if ( arg.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Check label name
            codeg::JumpPoint tmpPoint;
            tmpPoint._addressStatic = data._code.getCursor();
            tmpPoint._labelName = arg._str;

            if ( !data._jumps.addJumpPoint(tmpPoint) )
            {
                throw codeg::CompileError("bad label (unknown label \""+arg._str+"\")");
            }

            data._code.pushEmptyJump();
        }
        else if ( arg._type == codeg::KeywordTypes::KEYWORD_CONSTANT )
        {
            if ( arg._valueSize <= 3 )
            {
                data._code.pushFixedJump(arg._value);
            }
            else
            {
                throw codeg::ByteSizeError(1, "<= 3");
            }
        }
        else
        {
            throw codeg::ArgumentError(1, "name/constant");
        }
    }
    else if ( input._arguments.size() == 3)
    {//dynamic address position
        codeg::Keyword arg;
        if ( arg.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if ( arg._valueIsVariable )
            {//A variable
                arg._variable->_link.push_back({data._code.getCursor()});

                data._code.pushEmptyVarAccess();
                data._code.push(codeg::OPCODE_BJMPSRC3_CLK | codeg::READABLE_RAM);
                data._code.pushDummy();
            }
            else if ( arg._valueSize == 1 )
            {//A constant or readable bus
                data._code.push(codeg::OPCODE_BJMPSRC3_CLK | arg._valueBus);
                data._code.pushCheckDummy(arg._value, arg._valueBus);
            }
            else
            {
                throw codeg::ByteSizeError(1, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(1, "value");
        }

        if ( arg.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if ( arg._valueIsVariable )
            {//A variable
                arg._variable->_link.push_back({data._code.getCursor()});

                data._code.pushEmptyVarAccess();
                data._code.push(codeg::OPCODE_BJMPSRC2_CLK | codeg::READABLE_RAM);
                data._code.pushDummy();
            }
            else if ( arg._valueSize == 1 )
            {//A constant or readable bus
                data._code.push(codeg::OPCODE_BJMPSRC2_CLK | arg._valueBus);
                data._code.pushCheckDummy(arg._value, arg._valueBus);
            }
            else
            {
                throw codeg::ByteSizeError(2, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(2, "value");
        }

        if ( arg.process(input._arguments[2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if ( arg._valueIsVariable )
            {//A variable
                arg._variable->_link.push_back({data._code.getCursor()});

                data._code.pushEmptyVarAccess();
                data._code.push(codeg::OPCODE_BJMPSRC1_CLK | codeg::READABLE_RAM);
                data._code.pushDummy();
            }
            else if ( arg._valueSize == 1 )
            {//A constant or readable bus
                data._code.push(codeg::OPCODE_BJMPSRC1_CLK | arg._valueBus);
                data._code.pushCheckDummy(arg._value, arg._valueBus);
            }
            else
            {
                throw codeg::ByteSizeError(3, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(3, "value");
        }

        data._code.push(codeg::OPCODE_JMPSRC_CLK);
    }
    else
    {
        throw codeg::ArgumentsSizeError("1,3", input._arguments.size());
    }
}

///Instruction_restart
std::string Instruction_restart::getName() const
{
    return "restart";
}

void Instruction_restart::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 0 )
    {//Check size
        throw codeg::ArgumentsSizeError("0", input._arguments.size());
    }

    data._code.pushFixedJump(0);
}

///Instruction_affect
std::string Instruction_affect::getName() const
{
    return "affect";
}

void Instruction_affect::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() == 1 )
    {
        codeg::Keyword arg;
        if ( arg.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if (arg._valueIsVariable)
            {
                throw codeg::CompileError("can't copy a variable in another memory location");
            }
            if ( arg._valueSize > 1 )
            {
                throw codeg::ByteSizeError(1, "1");
            }

            data._code.push(codeg::OPCODE_RAMW | arg._valueBus);
            data._code.pushCheckDummy(arg._value, arg._valueBus);
        }
        else
        {
            throw codeg::ArgumentError(1, "value");
        }
    }
    else if ( input._arguments.size() == 2 )
    {//fixed specified address or variable
        codeg::Keyword arg;
        if ( arg.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {//A variable
            codeg::Keyword argValue;
            if ( argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
            {//Check value
                if ( !argValue._valueIsVariable )
                {
                    if (argValue._valueSize != 1)
                    {
                        throw codeg::ByteSizeError(2, "1");
                    }

                    arg._variable->_link.push_back({data._code.getCursor()});
                    data._code.pushEmptyVarAccess();

                    data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                    data._code.pushCheckDummy(argValue._value, argValue._valueBus);
                }
                else
                {
                    throw codeg::CompileError("can't copy a variable in another variable");
                }
            }
            else
            {
                throw codeg::ArgumentError(2, "value");
            }
        }
        else if ( arg._type == codeg::KeywordTypes::KEYWORD_CONSTANT )
        {//Constant
            if ( arg._valueSize > 2 )
            {
                throw codeg::ByteSizeError(1, "<= 2");
            }

            codeg::Keyword argValue;
            if ( argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
            {//A value
                if ( !argValue._valueIsVariable )
                {
                    if (argValue._valueSize != 1)
                    {
                        throw codeg::ByteSizeError(2, "1");
                    }

                    data._code.pushFixedVarAccess(arg._value);

                    data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                    data._code.pushCheckDummy(argValue._value, argValue._valueBus);
                }
                else
                {
                    throw codeg::CompileError("can't copy a variable in another variable");
                }
            }
            else
            {
                throw codeg::ArgumentError(2, "value");
            }
        }
        else
        {
            throw codeg::ArgumentError(1, "variable/constant");
        }
    }
    else if ( input._arguments.size() >= 3 )
    {
        codeg::Keyword argPoolName;
        if ( argPoolName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Pool name
            codeg::Pool* pool = data._pools.getPool(argPoolName._str);
            if (!pool)
            {
                throw codeg::CompileError("bad argument (unknown pool : \""+argPoolName._str+"\")");
            }
            if ( pool->getMaxSize() == 0 )
            {
                throw codeg::CompileError("bad argument (pool must have a fixed size)");
            }

            codeg::Keyword argOffset;
            if ( argOffset.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
            {//Offset
                if (argOffset._valueSize > 2)
                {
                    throw codeg::ByteSizeError(2, "<= 2");
                }

                codeg::MemorySize offset = argOffset._value;
                std::size_t numOfValue = input._arguments.size() - 2;
                if ( (numOfValue+offset) > pool->getMaxSize())
                {
                    throw codeg::CompileError("pool overflow (try to affect "+std::to_string(numOfValue)+" values with offset "+std::to_string(offset)+" but the max size is "+std::to_string(pool->getMaxSize())+")");
                }

                for (std::size_t i=0; i<numOfValue; ++i)
                {
                    codeg::Keyword argValue;
                    if ( argValue.process(input._arguments[i+2], codeg::KeywordTypes::KEYWORD_VALUE, data) )
                    {//Value
                        if ( !argValue._valueIsVariable )
                        {
                            if (argValue._valueSize != 1)
                            {
                                throw codeg::ByteSizeError(i+3, "1");
                            }

                            pool->_link.push_back({data._code.getCursor(), static_cast<codeg::MemorySize>(i+offset)});

                            data._code.pushEmptyVarAccess();

                            data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                            data._code.pushCheckDummy(argValue._value, argValue._valueBus);
                        }
                        else
                        {
                            throw codeg::CompileError("can't copy a variable in another location in memory");
                        }
                    }
                    else
                    {
                        throw codeg::ArgumentError(i+3, "value");
                    }
                }
            }
            else
            {
                throw codeg::ArgumentError(2, "constant");
            }
        }
        else
        {
            if (input._arguments.size() == 3)
            {
                codeg::Keyword arg;
                if ( arg.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
                {//A variable
                    codeg::MemorySize offset;
                    codeg::Keyword argOffset;
                    if ( argOffset.process(input._arguments[2], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
                    {//Offset
                        if (argOffset._valueSize > 2)
                        {
                            throw codeg::ByteSizeError(3, "<= 2");
                        }

                        offset = argOffset._value;
                    }
                    else
                    {
                        throw codeg::ArgumentError(3, "constant");
                    }

                    codeg::Keyword argValue;
                    if ( argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
                    {//Check value
                        if ( !argValue._valueIsVariable )
                        {
                            if (argValue._valueSize != 1)
                            {
                                throw codeg::ByteSizeError(2, "1");
                            }

                            arg._variable->_link.push_back({data._code.getCursor(), offset});
                            data._code.pushEmptyVarAccess();

                            data._code.push(codeg::OPCODE_RAMW | argValue._valueBus);
                            data._code.pushCheckDummy(argValue._value, argValue._valueBus);
                        }
                        else
                        {
                            throw codeg::CompileError("can't copy a variable in another variable");
                        }
                    }
                    else
                    {
                        throw codeg::ArgumentError(2, "value");
                    }
                }
                else
                {
                    throw codeg::ArgumentError(1, "name/variable");
                }
            }
            else
            {
                throw codeg::ArgumentError(1, "name");
            }
        }
    }
    else
    {
        throw codeg::ArgumentsSizeError("1,2, >= 3", input._arguments.size());
    }
}

///Instruction_get
std::string Instruction_get::getName() const
{
    return "get";
}

void Instruction_get::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() == 1 )
    {//fixed specified address or variable
        codeg::Keyword arg;
        if ( arg.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {//Variable
            arg._variable->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if ( arg._type == codeg::KeywordTypes::KEYWORD_CONSTANT )
        {//Constant
            if ( arg._valueSize > 2 )
            {
                throw codeg::ByteSizeError(1, "<= 2");
            }

            data._code.pushFixedVarAccess(arg._value);
        }
        else
        {
            throw codeg::ArgumentError(1, "variable/constant");
        }
    }
    else if ( input._arguments.size() == 2 )
    {//fixed size pool, or variable with offset
        codeg::Keyword argOffset;
        if ( argOffset.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
        {//Offset
            if (argOffset._valueSize > 2)
            {
                throw codeg::ByteSizeError(2, "<= 2");
            }
        }
        else
        {
            throw codeg::ArgumentError(2, "constant");
        }

        codeg::Keyword arg1;
        if ( arg1.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {//Pool name
            codeg::Pool* pool = data._pools.getPool(arg1._str);
            if (!pool)
            {
                throw codeg::CompileError("bad argument (unknown pool : \""+arg1._str+"\")");
            }
            if ( pool->getMaxSize() == 0 )
            {
                throw codeg::CompileError("bad argument (pool must have a fixed size)");
            }

            codeg::MemorySize offset = argOffset._value;
            if (offset >= pool->getMaxSize())
            {
                throw codeg::CompileError("pool overflow (try to get value at offset "+std::to_string(offset)+" but the max size is "+std::to_string(pool->getMaxSize())+")");
            }

            pool->_link.push_back({data._code.getCursor(), offset});
            data._code.pushEmptyVarAccess();
        }
        else
        {
            if (arg1._type == codeg::KeywordTypes::KEYWORD_VARIABLE)
            {//Variable
                codeg::MemorySize offset = argOffset._value;
                if (offset >= arg1._variable->_size)
                {
                    throw codeg::CompileError("variable overflow (try to get value at offset "+std::to_string(offset)+" but the variable size is "+std::to_string(arg1._variable->_size)+")");
                }

                arg1._variable->_link.push_back({data._code.getCursor(), offset});
                data._code.pushEmptyVarAccess();
            }
            else
            {
                throw codeg::ArgumentError(1, "name/variable");
            }
        }
    }
    else
    {
        throw codeg::ArgumentsSizeError("2,3", input._arguments.size());
    }
}

///Instruction_write
std::string Instruction_write::getName() const
{
    return "write";
}

void Instruction_write::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 2 )
    {//Check size
        throw codeg::ArgumentsSizeError("2", input._arguments.size());
    }

    codeg::Keyword argBus;
    if ( argBus.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_BUS, data) )
    {//A bus
        codeg::Keyword argValue;
        if ( argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if (argValue._valueIsVariable)
            {//A variable
                argValue._variable->_link.push_back({data._code.getCursor()});
                data._code.pushEmptyVarAccess();
            }
            else if (argValue._valueSize != 1)
            {
                throw codeg::ByteSizeError(2, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(2, "value");
        }

        switch ( argBus._value )
        {
        case codeg::BUS_NULL:
            throw codeg::CompileError("bad bus (bus can't be null)");
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
        case codeg::BUS_SPI:
            data._code.push(codeg::OPCODE_SPI_CLK | argValue._valueBus);
            break;

        case codeg::BUS_OPLEFT:
            data._code.push(codeg::OPCODE_OPLEFT_CLK | argValue._valueBus);
            break;
        case codeg::BUS_OPRIGHT:
            data._code.push(codeg::OPCODE_OPRIGHT_CLK | argValue._valueBus);
            break;

        default:
            throw codeg::CompileError("bad bus (unknown bus)");
            break;
        }
        data._code.pushCheckDummy(argValue._value, argValue._valueBus);
    }
    else
    {
        throw codeg::ArgumentError(1, "bus");
    }
}

///Instruction_select
std::string Instruction_select::getName() const
{
    return "select";
}

void Instruction_select::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 2 )
    {//Check size
        throw codeg::ArgumentsSizeError("2", input._arguments.size());
    }

    codeg::Keyword argTarget;
    if ( argTarget.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_TARGET, data) )
    {//A target
        codeg::Keyword argValue;
        if ( argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if (argValue._valueIsVariable)
            {//A variable
                argValue._variable->_link.push_back({data._code.getCursor()});
                data._code.pushEmptyVarAccess();
            }
            else if (argValue._valueSize != 1)
            {
                throw codeg::ByteSizeError(2, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(2, "value");
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
            throw codeg::CompileError("bad target (unknown target)");
            break;
        }
        data._code.pushCheckDummy(argValue._value, argValue._valueBus);
    }
    else
    {
        throw codeg::ArgumentError(1, "target");
    }
}

///Instruction_do
std::string Instruction_do::getName() const
{
    return "do";
}

void Instruction_do::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 3 && input._arguments.size() != 2 )
    {//Check size
        throw codeg::ArgumentsSizeError("2,3", input._arguments.size());
    }

    ///OPERATION
    std::size_t oprightArgIndex;
    if (input._arguments.size() == 3)
    {
        oprightArgIndex = 2;

        codeg::Keyword argValueOp;
        if ( argValueOp.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VALUE, data) )
        {//A value
            if (argValueOp._valueIsVariable)
            {
                argValueOp._variable->_link.push_back({data._code.getCursor()});
                data._code.pushEmptyVarAccess();
            }
            else if (argValueOp._valueSize != 1)
            {
                throw codeg::ByteSizeError(2, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(2, "value");
        }
        data._code.push(codeg::OPCODE_OPCHOOSE_CLK | argValueOp._valueBus);
        data._code.pushCheckDummy(argValueOp._value, argValueOp._valueBus);
    }
    else
    {
        oprightArgIndex = 1;
    }

    ///LEFT
    codeg::Keyword argValueLeft;
    if ( argValueLeft.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValueLeft._valueIsVariable)
        {
            argValueLeft._variable->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (argValueLeft._valueSize != 1)
        {
            throw codeg::ByteSizeError(1, "1");
        }
    }
    else
    {
        throw codeg::ArgumentError(1, "value");
    }
    data._code.push(codeg::OPCODE_OPLEFT_CLK | argValueLeft._valueBus);
    data._code.pushCheckDummy(argValueLeft._value, argValueLeft._valueBus);

    ///RIGHT
    codeg::Keyword argValueRight;
    if ( argValueRight.process(input._arguments[oprightArgIndex], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValueRight._valueIsVariable)
        {
            argValueRight._variable->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (argValueRight._valueSize != 1)
        {
            throw codeg::ByteSizeError(oprightArgIndex+1, "1");
        }
    }
    else
    {
        throw codeg::ArgumentError(oprightArgIndex+1, "value");
    }
    data._code.push(codeg::OPCODE_OPRIGHT_CLK | argValueRight._valueBus);
    data._code.pushCheckDummy(argValueRight._value, argValueRight._valueBus);
}

///Instruction_tick
std::string Instruction_tick::getName() const
{
    return "tick";
}

void Instruction_tick::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._arguments.size() != 1) && (input._arguments.size() != 2) )
    {//Check size
        throw codeg::ArgumentsSizeError("1-2", input._arguments.size());
    }

    codeg::Keyword argStr;
    if ( argStr.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_STRING, data) )
    {//A string
        if (argStr._str == "simple")
        {
            if ( input._arguments.size() == 2 )
            {//Check size
                codeg::Keyword argValue;
                if ( !argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
                {
                    throw codeg::ArgumentError(2, "constant");
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
            if ( input._arguments.size() == 2 )
            {//Check size
                codeg::Keyword argValue;
                if ( argValue.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
                {
                    throw codeg::ArgumentError(2, "constant");
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
            throw codeg::CompileError("bad argument (argument 1 [string] must be \"long\" or \"simple\")");
        }
    }
    else
    {
        throw codeg::ArgumentError(1, "string");
    }
}

///Instruction_brut
std::string Instruction_brut::getName() const
{
    return "brut";
}

void Instruction_brut::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() == 0 )
    {//Check size
        throw codeg::ArgumentsSizeError(">1", input._arguments.size());
    }

    codeg::Keyword argConstant;
    for (std::size_t i=0; i<input._arguments.size(); ++i)
    {
        if ( argConstant.process(input._arguments[i], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
        {//A value
            if ( argConstant._valueSize == 1 )
            {
                data._code.push(argConstant._value);
            }
            else
            {
                throw codeg::ByteSizeError(i+1, "1");
            }
        }
        else
        {
            throw codeg::ArgumentError(i+1, "constant");
        }
    }
}

///Instruction_function
std::string Instruction_function::getName() const
{
    return "function";
}

void Instruction_function::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 1 )
    {//Check size
        throw codeg::ArgumentsSizeError("1", input._arguments.size());
    }

    codeg::Keyword argName;
    if ( !argName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {
        throw codeg::ArgumentError(1, "name");
    }

    if ( data._functions.get(argName._str) != nullptr )
    {
        throw codeg::CompileError("bad function (function \""+argName._str+"\" already exist)");
    }

    if ( !data._actualFunctionName.empty() )
    {
        throw codeg::CompileError("function error (can't create a function in a function)");
    }
    if ( data._scopes.size() )
    {
        throw codeg::CompileError("function error (can't create a function in a scope)");
    }

    data._scopes.newScope(codeg::ScopeStats::SCOPE_FUNCTION, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._actualFunctionName = argName._str;
    data._functions.push(argName._str);

    data._jumps._jumpPoints.push_back({"%%E"+argName._str, data._code.getCursor()}); //Jump to the end of the function
    data._code.pushEmptyJump();

    if ( !data._jumps.addLabel({"%%"+argName._str, 0, data._code.getCursor()}) )
    {//Label to the start of the function
        throw codeg::CompileError("label error (label \"%%"+argName._str+"\" already exist)");
    }
}

///Instruction_if
std::string Instruction_if::getName() const
{
    return "if";
}

void Instruction_if::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 1 )
    {//Check size
        throw codeg::ArgumentsSizeError("1", input._arguments.size());
    }

    codeg::Keyword argValue;
    if ( argValue.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValue._valueIsVariable)
        {//A variable
            argValue._variable->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (argValue._valueSize != 1)
        {
            throw codeg::ByteSizeError(1, "1");
        }
    }
    else
    {
        throw codeg::ArgumentError(1, "value");
    }
    /*
    2 jump points must be created in order to work properly :
        - At the false condition, label named %%Fn where n is the scope
        - At the end of the condition, label named %%En where n is the scope
    if there is no "else" keyword, the label %%Fn will be the end of the condition
    */

    data._scopes.newScope(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopes.getScopeCount()), data._code.getCursor()});
    data._code.pushEmptyJumpAddress();

    data._code.push(codeg::OPCODE_IF | argValue._valueBus);
    data._code.pushCheckDummy(argValue._value, argValue._valueBus);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_else
std::string Instruction_else::getName() const
{
    return "else";
}

void Instruction_else::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 0 )
    {//Check size
        throw codeg::ArgumentsSizeError("0", input._arguments.size());
    }

    if ( data._scopes.empty() )
    {
        throw codeg::CompileError("scope error (else must be placed in a valid conditional scope)");
    }

    if ( data._scopes.top()._stat != codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE )
    {
        throw codeg::CompileError("scope error (else must be placed after a conditional keyword)");
    }

    data._jumps._jumpPoints.push_back({"%%E"+std::to_string(data._scopes.top()._id), data._code.getCursor()});
    data._code.pushEmptyJump();

    if ( !data._jumps.addLabel({"%%F"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
    {
        throw codeg::CompileError("else : label error (label \"%%F"+std::to_string(data._scopes.top()._id)+"\" already exist)");
    }

    data._scopes.top()._stat = codeg::ScopeStats::SCOPE_CONDITIONAL_FALSE;
}

///Instruction_ifnot
std::string Instruction_ifnot::getName() const
{
    return "if_not";
}

void Instruction_ifnot::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 1 )
    {//Check size
        throw codeg::ArgumentsSizeError("1", input._arguments.size());
    }

    codeg::Keyword argValue;
    if ( argValue.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_VALUE, data) )
    {//A value
        if (argValue._valueIsVariable)
        {//A variable
            argValue._variable->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (argValue._valueSize != 1)
        {
            throw codeg::ByteSizeError(1, "1");
        }
    }
    else
    {
        throw codeg::ArgumentError(1, "value");
    }
    /*
    2 jump points must be created in order to work properly :
        - At the false condition, label named %%Fn where n is the scope
        - At the end of the condition, label named %%En where n is the scope
    if there is no "else" keyword, the label %%Fn will be the end of the condition
    */

    data._scopes.newScope(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopes.getScopeCount()), data._code.getCursor()});
    data._code.pushEmptyJumpAddress();

    data._code.push(codeg::OPCODE_IFNOT | argValue._valueBus);
    data._code.pushCheckDummy(argValue._value, argValue._valueBus);
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_end
std::string Instruction_end::getName() const
{
    return "end";
}

void Instruction_end::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 0 )
    {//Check size
        throw codeg::ArgumentsSizeError("0", input._arguments.size());
    }

    if ( data._scopes.empty() )
    {
        throw codeg::CompileError("scope error ('end' must be placed to end a scope)");
    }

    //Ending a scope
    switch ( data._scopes.top()._stat )
    {
    case codeg::ScopeStats::SCOPE_FUNCTION:
        //Ending a function
        if ( !data._jumps.addLabel({"%%E"+data._actualFunctionName, 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("label error (label \"%%E"+data._actualFunctionName+"\" already exist)");
        }
        data._actualFunctionName.clear();
        break;
    case codeg::ScopeStats::SCOPE_CONDITIONAL_FALSE:
        //Ending a conditional scope with the "else" keyword
        if ( !data._jumps.addLabel({"%%E"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("label error (label \"%%E"+std::to_string(data._scopes.top()._id)+"\" already exist)");
        }
        break;
    case codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE:
        //Ending a conditional scope without the "else" keyword
        if ( !data._jumps.addLabel({"%%F"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("label error (label \"%%F"+std::to_string(data._scopes.top()._id)+"\" already exist)");
        }
        if ( !data._jumps.addLabel({"%%E"+std::to_string(data._scopes.top()._id), 0, data._code.getCursor()}) )
        {
            throw codeg::CompileError("label error (label \"%%E"+std::to_string(data._scopes.top()._id)+"\" already exist)");
        }
        break;
    default:
        throw codeg::FatalError("Bad scope stat !");
        break;
    }

    data._scopes.pop();
}

///Instruction_call
std::string Instruction_call::getName() const
{
    return "call";
}

void Instruction_call::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() == 4 )
    {//call a function
        codeg::Keyword argName;
        if ( !argName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {
            throw codeg::ArgumentError(1, "name");
        }
        codeg::Function* func = data._functions.get(argName._str);
        if ( func == nullptr )
        {
            throw codeg::CompileError("bad function (unknown function \""+argName._str+"\")");
        }
        if ( func->isDefinition() )
        {
            throw codeg::CompileError("bad function (can't call a definition with a return address)");
        }

        codeg::Keyword argVar1;
        if ( !argVar1.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {
            throw codeg::ArgumentError(2, "variable");
        }
        codeg::Keyword argVar2;
        if ( !argVar2.process(input._arguments[2], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {
            throw codeg::ArgumentError(3, "variable");
        }
        codeg::Keyword argVar3;
        if ( !argVar3.process(input._arguments[3], codeg::KeywordTypes::KEYWORD_VARIABLE, data) )
        {
            throw codeg::ArgumentError(4, "variable");
        }

        //Prepare return address
        uint32_t returnAddress = data._code.getCursor() + 25;

        argVar1._variable->_link.push_back({data._code.getCursor()}); //MSB
        data._code.pushEmptyVarAccess();
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push((returnAddress&0x00FF0000)>>16);

        argVar2._variable->_link.push_back({data._code.getCursor()}); //MSB
        data._code.pushEmptyVarAccess();
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push((returnAddress&0x0000FF00)>>8);

        argVar3._variable->_link.push_back({data._code.getCursor()}); //MSB
        data._code.pushEmptyVarAccess();
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push(returnAddress&0x000000FF);

        codeg::JumpPoint tmpPoint;
        tmpPoint._addressStatic = data._code.getCursor();
        tmpPoint._labelName = "%%"+argName._str;

        if ( !data._jumps.addJumpPoint(tmpPoint) )
        {
            throw codeg::CompileError("bad label (unknown label \"%%"+argName._str+"\")");
        }
        data._code.pushEmptyJump();
    }
    else if ( input._arguments.size() == 1 )
    {//call a definition/function
        codeg::Keyword argName;
        if ( !argName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
        {
            throw codeg::ArgumentError(1, "name");
        }
        codeg::Function* func = data._functions.get(argName._str);
        if ( func == nullptr )
        {
            throw codeg::CompileError("bad definition/function (unknown definition/function \""+argName._str+"\")");
        }
        if ( func->isDefinition() )
        {//definition
            data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_definition(func)) );
        }
        else
        {//function
            codeg::JumpPoint tmpPoint;
            tmpPoint._addressStatic = data._code.getCursor();
            tmpPoint._labelName = "%%"+argName._str;

            if ( !data._jumps.addJumpPoint(tmpPoint) )
            {
                throw codeg::CompileError("bad label (unknown label \"%%"+argName._str+"\")");
            }
            data._code.pushEmptyJump();
        }
    }
    else
    {
        throw codeg::ArgumentsSizeError("4,1", input._arguments.size());
    }
}

///Instruction_clock
std::string Instruction_clock::getName() const
{
    return "clock";
}

void Instruction_clock::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._arguments.size() != 1) && (input._arguments.size() != 2) )
    {//Check size
        throw codeg::ArgumentsSizeError("1,2", input._arguments.size());
    }

    codeg::Keyword argTarget;
    if ( !argTarget.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_TARGET, data) )
    {
        throw codeg::ArgumentError(1, "target");
    }

    uint8_t targetOpcode;
    switch (argTarget._target)
    {
    case codeg::TARGET_PERIPHERAL:
        targetOpcode = codeg::OPCODE_PERIPHERAL_CLK;
        break;
    default:
        throw codeg::ArgumentError(1, "target");
        break;
    }

    if (input._arguments.size() == 2)
    {
        codeg::Keyword argConstant;
        if ( argConstant.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
        {//A value
            if (argConstant._value == 0)
            {
                codeg::ConsoleWarningWrite("you want 0 clock pulses, this instruction will be ignored");
            }

            for (uint32_t i=0; i<argConstant._value; ++i)
            {
                data._code.push(targetOpcode | codeg::READABLE_DEFAULT);
                data._code.pushDummy();
            }
        }
        else
        {
            throw codeg::ArgumentError(2, "constant");
        }
    }
    else
    {
        data._code.push(targetOpcode | codeg::READABLE_DEFAULT);
        data._code.pushDummy();
    }
}

///Instruction_pool
std::string Instruction_pool::getName() const
{
    return "pool";
}

void Instruction_pool::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( (input._arguments.size() != 2) && (input._arguments.size() != 3) )
    {//Check size
        throw codeg::ArgumentsSizeError("2-3", input._arguments.size());
    }

    codeg::Keyword argName;
    if ( !argName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {//A name
        throw codeg::ArgumentError(1, "name");
    }

    codeg::Keyword argSize;
    if ( !argSize.process(input._arguments[1], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
    {//A constant
        throw codeg::ArgumentError(2, "constant");
    }

    codeg::Address startAddress = 0;
    bool isDynamic = true;
    if (input._arguments.size() == 3)
    {
        codeg::Keyword argStart;
        if ( !argStart.process(input._arguments[2], codeg::KeywordTypes::KEYWORD_CONSTANT, data) )
        {//A constant
            throw codeg::ArgumentError(3, "constant");
        }
        startAddress = argStart._value;
        isDynamic = false;
    }

    if ( codeg::Pool* tmpPool = data._pools.getPool(argName._str) )
    {//Pool already exist
        codeg::ConsoleWarningWrite("pool \""+argName._str+"\" already exist and will be replaced !");

        tmpPool->setStartAddressType(isDynamic ? codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC : codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC);
        tmpPool->setAddress(startAddress, argSize._value);
    }
    else
    {//Pool must be created
        codeg::Pool tmpNewPool(argName._str);

        tmpNewPool.setStartAddressType(isDynamic ? codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC : codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC);
        tmpNewPool.setAddress(startAddress, argSize._value);

        if ( !data._pools.addPool(tmpNewPool) )
        {
            throw codeg::FatalError("bad pool (pool \""+argName._str+"\" already exist ?)");
        }
    }
}

///Instruction_import
std::string Instruction_import::getName() const
{
    return "import";
}

void Instruction_import::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 1 )
    {//Check size
        throw codeg::ArgumentsSizeError("1", input._arguments.size());
    }

    std::string path = data._relativePath + input._arguments[0];

    if ( !data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_file(path)) ) )
    {
        throw codeg::FatalError("can't open the file : \""+path+"\"");
    }
}

///Instruction_definition
std::string Instruction_definition::getName() const
{
    return "definition";
}

void Instruction_definition::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 1 )
    {//Check size
        throw codeg::ArgumentsSizeError("1", input._arguments.size());
    }

    codeg::Keyword argName;
    if ( !argName.process(input._arguments[0], codeg::KeywordTypes::KEYWORD_NAME, data) )
    {
        throw codeg::ArgumentError(1, "name");
    }

    if ( data._functions.get(argName._str) != nullptr )
    {
        throw codeg::CompileError("bad definition (definition/function \""+argName._str+"\" already exist)");
    }

    if ( data._scopes.size() )
    {
        throw codeg::CompileError("definition error (can't create a definition in a scope)");
    }

    data._scopes.newScope(codeg::ScopeStats::SCOPE_DEFINITION, data._reader.getlineCount(), data._reader.getPath()); //New scope

    data._actualFunctionName = argName._str;
    data._functions.push(argName._str, true);

    data._writeLinesIntoDefinition = true;
}

///Instruction_enddef
std::string Instruction_enddef::getName() const
{
    return "end_def";
}

void Instruction_enddef::compile( [[maybe_unused]] const codeg::StringDecomposer& input, [[maybe_unused]] codeg::CompilerData& data)
{
    throw codeg::CompileError("'end_def' can only be placed to end a definition scope");
}
void Instruction_enddef::compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if ( input._arguments.size() != 0 )
    {//Check size
        throw codeg::ArgumentsSizeError("0", input._arguments.size());
    }

    //Ending the definition
    data._writeLinesIntoDefinition = false;
    data._actualFunctionName.clear();

    data._scopes.pop();
}

}//end codeg
