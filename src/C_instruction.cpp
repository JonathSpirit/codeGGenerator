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
    auto kStringName = KeywordString::parse(input._arguments[0], data);
    auto kStringValue = KeywordString::parse(input._arguments[1], data);

    if (!kStringName.has_value())
    {
        throw codeg::ArgumentError(1, "string");
    }

    if (!kStringValue.has_value())
    {
        throw codeg::ArgumentError(2, "string");
    }

    if ( data._macros.check(kStringName->getString()) )
    {//Check if already set
        data._macros.set(kStringName->getString(), kStringValue->getString());
        ConsoleWarning << "macro \"" << kStringName->getString() << "\" already exist and will be replaced" << std::endl;
    }
    else
    {
        data._macros.set(kStringName->getString(), kStringValue->getString());
    }
}

///Instruction_unset
std::string Instruction_unset::getName() const
{
    return "unset";
}

void Instruction_unset::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kStringName = KeywordString::parse(input._arguments[0], data);

    if ( !kStringName.has_value() )
    {
        throw codeg::ArgumentError(1, "string");
    }

    data._macros.remove(kStringName->getString());
}

///Instruction_var
std::string Instruction_var::getName() const
{
    return "var";
}

void Instruction_var::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    codeg::MemorySize wantedMemorySize = 1;
    auto kVariable = KeywordVariable::parse(input._arguments[0], data);

    if (!kVariable.has_value())
    {
        throw codeg::ArgumentError(1, "variable");
    }

    if (input._arguments.size() == 2)
    {
        auto kConstant = KeywordConstant::parse(input._arguments[1], data);
        if (!kConstant.has_value())
        {
            throw codeg::ArgumentError(2, "constant");
        }
        if ( (kConstant->getValue()->first == 0) ||
             (kConstant->getValue()->first > std::numeric_limits<codeg::MemorySize>::max()) )
        {
            throw codeg::CompileError("variable size cannot be 0 or >"+std::to_string(std::numeric_limits<codeg::MemorySize>::max()));
        }
        wantedMemorySize = kConstant->getValue()->first;
    }

    if (kVariable->exist())
    {
        if (kVariable->getVariable()->_size == wantedMemorySize)
        {
            ConsoleWarning << "redeclaration of variable \""
                           << kVariable->getName()
                           << "\" in pool \""
                           << kVariable->getPoolName()
                           << "\" with size of " << wantedMemorySize << std::endl;
        }
        else
        {
            throw codeg::CompileError("redeclaration of variable \""+kVariable->getName()+
                "\" in pool \""+kVariable->getPoolName()+
                "\" but with different size (wanted "+std::to_string(wantedMemorySize)+
                " instead of "+std::to_string(kVariable->getVariable()->_size)+")");
        }
    }
    else
    {
        if ( codeg::Pool* pool = data._pools.getPool(kVariable->getPoolName()) )
        {//Check pool
            if ( !pool->addVariable(kVariable->getName(), wantedMemorySize) )
            {
                throw codeg::FatalError("unknown error, variable should be ok but can't create it");
            }
        }
        else
        {
            throw codeg::CompileError("bad pool (pool \""+kVariable->getPoolName()+"\" does not exist)");
        }
    }
}

///Instruction_label
std::string Instruction_label::getName() const
{
    return "label";
}

void Instruction_label::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    codeg::Address address = data._code.getCursor();

    auto kName = KeywordName::parse(input._arguments[0], data);

    if ( !kName.has_value() )
    {
        throw codeg::ArgumentError(1, "name");
    }

    if ( input._arguments.size() == 2)
    {//Custom address position
        auto kConstant = KeywordConstant::parse(input._arguments[1], data);

        if ( !kConstant.has_value() )
        {
            throw codeg::ArgumentError(2, "constant");
        }

        address = kConstant->getValue()->first;
    }

    codeg::Label tmpLabel;
    tmpLabel._addressStatic = address;
    tmpLabel._uniqueIndex = 0;
    tmpLabel._name = kName->getName();

    if ( !data._jumps.addLabel(tmpLabel) )
    {//Check label
        throw codeg::CompileError("bad label (label \""+kName->getName()+"\" already exist)");
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
    {//Label name or fixed address
        auto kConstant = KeywordConstant::parse(input._arguments[0], data);

        if (kConstant.has_value())
        {//Fixed address
            if ( kConstant->getValue()->second <= 3 )
            {
                data._code.pushFixedJump(kConstant->getValue()->first);
            }
            else
            {
                throw codeg::ByteSizeError(1, "<= 3");
            }
        }
        else
        {
            auto kName = KeywordName::parse(input._arguments[0], data);

            if (kName.has_value())
            {//Label name
                codeg::JumpPoint tmpPoint;
                tmpPoint._addressStatic = data._code.getCursor();
                tmpPoint._labelName = kName->getName();

                if ( !data._jumps.addJumpPoint(tmpPoint) )
                {
                    throw codeg::CompileError("bad label (unknown label \""+kName->getName()+"\")");
                }

                data._code.pushEmptyJump();
            }
            else
            {
                throw codeg::ArgumentError(1, "name/constant");
            }
        }
    }
    else if ( input._arguments.size() == 3)
    {//Dynamic address position
        const std::pair<uint32_t, std::size_t> defaultValue{0,1};

        auto kValueMSB = KeywordValue::parse(input._arguments[0], data);
        auto kValue = KeywordValue::parse(input._arguments[1], data);
        auto kValueLSB = KeywordValue::parse(input._arguments[2], data);

        if (!kValueMSB.has_value())
        {
            throw codeg::ArgumentError(1, "value");
        }
        if (!kValue.has_value())
        {
            throw codeg::ArgumentError(2, "value");
        }
        if (!kValueLSB.has_value())
        {
            throw codeg::ArgumentError(3, "value");
        }

        if (kValueMSB->isVariable())
        {
            kValueMSB->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (kValueMSB->getValue().value_or(defaultValue).second != 1)
        {
            throw codeg::ByteSizeError(1, "1");
        }

        data._code.push(codeg::OPCODE_BJMPSRC3_CLK | kValueMSB->getReadableBusType().value());
        data._code.pushCheckDummy(kValueMSB->getValue().value_or(defaultValue).first, kValueMSB->getReadableBusType().value());

        if (kValue->isVariable())
        {
            kValue->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (kValue->getValue().value_or(defaultValue).second != 1)
        {
            throw codeg::ByteSizeError(2, "1");
        }

        data._code.push(codeg::OPCODE_BJMPSRC2_CLK | kValue->getReadableBusType().value());
        data._code.pushCheckDummy(kValue->getValue().value_or(defaultValue).first, kValue->getReadableBusType().value());

        if (kValueLSB->isVariable())
        {
            kValueLSB->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else if (kValueLSB->getValue().value_or(defaultValue).second != 1)
        {
            throw codeg::ByteSizeError(3, "1");
        }

        data._code.push(codeg::OPCODE_BJMPSRC1_CLK | kValueLSB->getReadableBusType().value());
        data._code.pushCheckDummy(kValueLSB->getValue().value_or(defaultValue).first, kValueLSB->getReadableBusType().value());

        data._code.push(codeg::OPCODE_JMPSRC_CLK);
    }
}

///Instruction_restart
std::string Instruction_restart::getName() const
{
    return "restart";
}

void Instruction_restart::compile([[maybe_unused]] const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
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
    {//assigning without setting the address
        auto kValue = KeywordValue::parse(input._arguments[0], data);

        if (!kValue.has_value())
        {
            throw codeg::ArgumentError(1, "value");
        }

        if (kValue->isVariable())
        {
            throw codeg::CompileError("can't copy a variable in another memory location");
        }
        auto value = kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1});
        if (value.second != 1)
        {
            throw codeg::ByteSizeError(1, "1");
        }

        data._code.push(codeg::OPCODE_RAMW | kValue->getReadableBusType().value());
        data._code.pushCheckDummy(value.first, kValue->getReadableBusType().value());
    }
    else if ( input._arguments.size() == 2)
    {//fixed specified address or variable
        auto kConstant = KeywordConstant::parse(input._arguments[0], data);
        auto kValue = KeywordValue::parse(input._arguments[1], data);

        if (!kValue.has_value())
        {
            throw codeg::ArgumentError(2, "value");
        }
        if (kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).second != 1)
        {
            throw codeg::ByteSizeError(2, "1");
        }

        if (!kConstant.has_value())
        {
            auto kVariable = KeywordVariable::parse(input._arguments[0], data, true);
            if (!kVariable.has_value())
            {
                throw codeg::ArgumentError(1, "variable/constant");
            }
            else
            {
                //variable
                if (kValue->isVariable())
                {
                    throw codeg::CompileError("can't copy a variable in another variable");
                }

                kVariable->getVariable()->_link.push_back({data._code.getCursor()});
                data._code.pushEmptyVarAccess();

                data._code.push(codeg::OPCODE_RAMW | kValue->getReadableBusType().value());
                data._code.pushCheckDummy(kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).first, kValue->getReadableBusType().value());
            }
        }
        else
        {
            //fixed specified address
            if (kValue->isVariable())
            {
                throw codeg::CompileError("can't copy a variable in another memory location");
            }

            if (kConstant->getValue()->second > 2)
            {
                throw codeg::ByteSizeError(1, "<= 2");
            }

            data._code.pushFixedVarAccess(kConstant->getValue()->first);

            data._code.push(codeg::OPCODE_RAMW | kValue->getReadableBusType().value());
            data._code.pushCheckDummy(kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).first, kValue->getReadableBusType().value());
        }
    }
    else if ( input._arguments.size() >= 3 )
    {//Assigns in a fixed size pool or a variable with an offset
        auto kNamePoolName = KeywordName::parse(input._arguments[0], data);

        if (kNamePoolName.has_value())
        {//Assigns in a fixed size pool
            auto kConstantOffset = KeywordConstant::parse(input._arguments[1], data);

            if (!kConstantOffset.has_value())
            {
                throw codeg::ArgumentError(2, "constant");
            }
            if (kConstantOffset->getValue()->second > 2)
            {
                throw codeg::ByteSizeError(2, "<= 2");
            }

            codeg::Pool* pool = data._pools.getPool(kNamePoolName->getName());
            if (pool == nullptr)
            {
                throw codeg::CompileError("bad argument (unknown pool : \""+kNamePoolName->getName()+"\")");
            }
            if ( pool->getMaxSize() == 0 )
            {
                throw codeg::CompileError("bad argument (pool must have a fixed size)");
            }

            codeg::MemorySize offset = kConstantOffset->getValue()->first;
            std::size_t numOfValue = input._arguments.size() - 2;
            if ( (numOfValue+offset) > pool->getMaxSize())
            {
                throw codeg::CompileError("pool overflow (try to affect "+std::to_string(numOfValue)+" values with offset "+std::to_string(offset)+" but the max size is "+std::to_string(pool->getMaxSize())+")");
            }

            for (std::size_t i=0; i<numOfValue; ++i)
            {
                auto kValue = KeywordValue::parse(input._arguments[i+2], data);

                if (!kValue.has_value())
                {
                    throw codeg::ArgumentError(i+3, "value");
                }
                if (kValue->isVariable())
                {
                    throw codeg::CompileError("can't copy a variable in another location in memory");
                }
                if (kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).second != 1)
                {
                    throw codeg::ByteSizeError(i+3, "1");
                }

                pool->_link.push_back({data._code.getCursor(), static_cast<codeg::MemorySize>(i+offset)});

                data._code.pushEmptyVarAccess();

                data._code.push(codeg::OPCODE_RAMW | kValue->getReadableBusType().value());
                data._code.pushCheckDummy(kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).first, kValue->getReadableBusType().value());
            }
        }
        else if (input._arguments.size() == 3)
        {//Assigns a variable with an offset
            auto kConstant = KeywordConstant::parse(input._arguments[2], data);
            auto kValue = KeywordValue::parse(input._arguments[1], data);
            auto kVariable = KeywordVariable::parse(input._arguments[0], data, true);

            if (!kVariable.has_value())
            {
                throw codeg::ArgumentError(1, "variable/name");
            }
            if (!kValue.has_value())
            {
                throw codeg::ArgumentError(2, "value");
            }
            if (!kConstant.has_value())
            {
                throw codeg::ArgumentError(3, "constant");
            }

            if (kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).second != 1)
            {
                throw codeg::ByteSizeError(2, "1");
            }
            if (kConstant->getValue()->second > 2)
            {
                throw codeg::ByteSizeError(3, "<= 2");
            }

            if (kValue->isVariable())
            {
                throw codeg::CompileError("can't copy a variable in another variable");
            }

            codeg::MemorySize offset = kConstant->getValue()->first;

            kVariable->getVariable()->_link.push_back({data._code.getCursor(), offset});
            data._code.pushEmptyVarAccess();

            data._code.push(codeg::OPCODE_RAMW | kValue->getReadableBusType().value());
            data._code.pushCheckDummy(kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1}).first, kValue->getReadableBusType().value());
        }
        else
        {
            throw codeg::ArgumentError(1, "name");
        }
    }
}

///Instruction_get
std::string Instruction_get::getName() const
{
    return "get";
}

void Instruction_get::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    if (input._arguments.size() == 1)
    {//fixed specified address or variable
        auto kConstant = KeywordConstant::parse(input._arguments[0], data);

        if (!kConstant.has_value())
        {
            auto kVariable = KeywordVariable::parse(input._arguments[0], data, true);

            if (!kVariable.has_value())
            {
                throw codeg::ArgumentError(1, "variable/constant");
            }
            //variable
            kVariable->getVariable()->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        else
        {
            //fixed specified address
            if (kConstant->getValue()->second > 2)
            {
                throw codeg::ByteSizeError(1, "<= 2");
            }
            data._code.pushFixedVarAccess(kConstant->getValue()->first);
        }
    }
    else if (input._arguments.size() == 2)
    {//fixed size pool, or variable with offset
        auto kVariable = KeywordVariable::parse(input._arguments[0], data, true);
        auto kConstantOffset = KeywordConstant::parse(input._arguments[1], data);

        if (!kConstantOffset.has_value())
        {
            throw codeg::ArgumentError(2, "constant");
        }
        if (kConstantOffset->getValue()->second > 2)
        {
            throw codeg::ByteSizeError(2, "<= 2");
        }

        codeg::MemorySize offset = kConstantOffset->getValue()->first;

        if (!kVariable.has_value())
        {
            auto kName = KeywordName::parse(input._arguments[0], data);

            if (!kName.has_value())
            {
                throw codeg::ArgumentError(1, "name/variable");
            }
            //fixed size pool
            codeg::Pool* pool = data._pools.getPool(kName->getName());
            if (!pool)
            {
                throw codeg::CompileError("bad argument (unknown pool : \""+kName->getName()+"\")");
            }
            if ( pool->getMaxSize() == 0 )
            {
                throw codeg::CompileError("bad argument (pool must have a fixed size)");
            }

            pool->_link.push_back({data._code.getCursor(), offset});
            data._code.pushEmptyVarAccess();
        }
        else
        {
            //variable with offset
            if (offset >= kVariable->getVariable()->_size)
            {
                throw codeg::CompileError("variable overflow (try to get value at offset "+std::to_string(offset)+" but the variable size is "+std::to_string(kVariable->getVariable()->_size)+")");
            }

            kVariable->getVariable()->_link.push_back({data._code.getCursor(), offset});
            data._code.pushEmptyVarAccess();
        }
    }
}

///Instruction_write
std::string Instruction_write::getName() const
{
    return "write";
}

void Instruction_write::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kBus = KeywordBus::parse(input._arguments[0], data);
    auto kValue = KeywordValue::parse(input._arguments[1], data);

    if (!kBus.has_value())
    {
        throw codeg::ArgumentError(1, "bus");
    }
    if (!kValue.has_value())
    {
        throw codeg::ArgumentError(2, "value");
    }

    if (kValue->isVariable())
    {
        kValue->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
        data._code.pushEmptyVarAccess();
    }
    auto value = kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1});
    if (value.second != 1)
    {
        throw codeg::ByteSizeError(2, "1");
    }

    switch (kBus->getBus())
    {
    case codeg::BUS_WRITEABLE_1:
        data._code.push(codeg::OPCODE_BWRITE1_CLK | kValue->getReadableBusType().value());
        break;
    case codeg::BUS_WRITEABLE_2:
        data._code.push(codeg::OPCODE_BWRITE2_CLK | kValue->getReadableBusType().value());
        break;
    case codeg::BUS_SPICFG:
        data._code.push(codeg::OPCODE_BCFG_SPI_CLK | kValue->getReadableBusType().value());
        break;
    case codeg::BUS_SPI:
        data._code.push(codeg::OPCODE_SPI_CLK | kValue->getReadableBusType().value());
        break;

    case codeg::BUS_OPLEFT:
        data._code.push(codeg::OPCODE_OPLEFT_CLK | kValue->getReadableBusType().value());
        break;
    case codeg::BUS_OPRIGHT:
        data._code.push(codeg::OPCODE_OPRIGHT_CLK | kValue->getReadableBusType().value());
        break;

    default:
        throw codeg::CompileError("bad bus (unknown bus)");
    }

    data._code.pushCheckDummy(value.first, kValue->getReadableBusType().value());
}

///Instruction_select
std::string Instruction_select::getName() const
{
    return "select";
}

void Instruction_select::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kTarget = KeywordTarget::parse(input._arguments[0], data);
    auto kValue = KeywordValue::parse(input._arguments[1], data);

    if (!kTarget.has_value())
    {
        throw codeg::ArgumentError(1, "target");
    }
    if (!kValue.has_value())
    {
        throw codeg::ArgumentError(2, "value");
    }

    if (kValue->isVariable())
    {
        kValue->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
        data._code.pushEmptyVarAccess();
    }
    auto value = kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1});
    if (value.second != 1)
    {
        throw codeg::ByteSizeError(2, "1");
    }

    switch (kTarget->getTarget())
    {
    case codeg::TargetType::TARGET_OPERATION:
        data._code.push(codeg::OPCODE_OPCHOOSE_CLK | kValue->getReadableBusType().value());
        break;
    case codeg::TargetType::TARGET_PERIPHERAL:
        data._code.push(codeg::OPCODE_BPCS_CLK | kValue->getReadableBusType().value());
        break;

    default:
        throw codeg::CompileError("bad target (unknown target)");
    }
    data._code.pushCheckDummy(value.first, kValue->getReadableBusType().value());
}

///Instruction_do
std::string Instruction_do::getName() const
{
    return "do";
}

void Instruction_do::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    const std::pair<uint32_t,std::size_t> defaultValue{0,1};
    std::optional<KeywordValue> kValueOpLeft;
    std::optional<KeywordValue> kValueOpRight;

    //Operation choose
    if (input._arguments.size() == 3)
    {
        kValueOpLeft = KeywordValue::parse(input._arguments[0], data);
        kValueOpRight = KeywordValue::parse(input._arguments[2], data);

        if (!kValueOpLeft.has_value())
        {
            throw codeg::ArgumentError(1, "value");
        }
        if (!kValueOpRight.has_value())
        {
            throw codeg::ArgumentError(3, "value");
        }

        auto kValueOpChoose = KeywordValue::parse(input._arguments[1], data);

        if (!kValueOpChoose.has_value())
        {
            throw codeg::ArgumentError(2, "value");
        }
        if (kValueOpChoose->isVariable())
        {
            kValueOpChoose->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
            data._code.pushEmptyVarAccess();
        }
        if (kValueOpChoose->getValue().value_or(defaultValue).second != 1)
        {
            throw codeg::ByteSizeError(2, "1");
        }

        data._code.push(codeg::OPCODE_OPCHOOSE_CLK | kValueOpChoose->getReadableBusType().value());
        data._code.pushCheckDummy(kValueOpChoose->getValue().value_or(defaultValue).first, kValueOpChoose->getReadableBusType().value());
    }
    else
    {
        kValueOpLeft = KeywordValue::parse(input._arguments[0], data);
        kValueOpRight = KeywordValue::parse(input._arguments[1], data);

        if (!kValueOpLeft.has_value())
        {
            throw codeg::ArgumentError(1, "value");
        }
        if (!kValueOpRight.has_value())
        {
            throw codeg::ArgumentError(2, "value");
        }
    }

    //Left value
    if (kValueOpLeft->isVariable())
    {
        kValueOpLeft->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
        data._code.pushEmptyVarAccess();
    }
    if (kValueOpLeft->getValue().value_or(defaultValue).second != 1)
    {
        throw codeg::ByteSizeError(1, "1");
    }

    data._code.push(codeg::OPCODE_OPLEFT_CLK | kValueOpLeft->getReadableBusType().value());
    data._code.pushCheckDummy(kValueOpLeft->getValue().value_or(defaultValue).first, kValueOpLeft->getReadableBusType().value());

    //Right value
    if (kValueOpRight->isVariable())
    {
        kValueOpRight->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
        data._code.pushEmptyVarAccess();
    }
    if (kValueOpRight->getValue().value_or(defaultValue).second != 1)
    {
        throw codeg::ByteSizeError(1 + (input._arguments.size() == 3 ? 1 : 0), "1");
    }

    data._code.push(codeg::OPCODE_OPRIGHT_CLK | kValueOpRight->getReadableBusType().value());
    data._code.pushCheckDummy(kValueOpRight->getValue().value_or(defaultValue).first, kValueOpRight->getReadableBusType().value());
}

///Instruction_tick
std::string Instruction_tick::getName() const
{
    return "tick";
}

void Instruction_tick::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kString = KeywordString::parse(input._arguments[0], data);
    codeg::MemoryBigSize count = 1;

    if (input._arguments.size() == 2)
    {
        auto kConstant = KeywordConstant::parse(input._arguments[1], data);
        if (!kConstant.has_value())
        {
            throw codeg::ArgumentError(2, "constant");
        }
        count = kConstant->getValue()->first;
    }

    if (!kString.has_value())
    {
        throw codeg::ArgumentError(1, "string");
    }

    if (kString->getString() == "simple")
    {
        for (codeg::MemoryBigSize i=0; i<count; ++i)
        {
            data._code.push(codeg::OPCODE_STICK | codeg::READABLE_DEFAULT);
            data._code.pushCheckDummy(0, codeg::READABLE_DEFAULT);
        }
    }
    else if (kString->getString() == "long")
    {
        for (codeg::MemoryBigSize i=0; i<count; ++i)
        {
            data._code.push(codeg::OPCODE_LTICK | codeg::READABLE_DEFAULT);
            data._code.pushCheckDummy(0, codeg::READABLE_DEFAULT);
        }
    }
    else
    {
        throw codeg::CompileError("bad argument (argument 1 [string] must be \"long\" or \"simple\")");
    }
}

///Instruction_brut
std::string Instruction_brut::getName() const
{
    return "brut";
}

void Instruction_brut::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    for (std::size_t i=0; i<input._arguments.size(); ++i)
    {
        auto kConstant = KeywordConstant::parse(input._arguments[i], data);

        if (!kConstant.has_value())
        {
            throw codeg::ArgumentError(i+1, "constant");
        }
        if (kConstant->getValue()->second != 1)
        {
            throw codeg::ByteSizeError(i+1, "1");
        }

        data._code.push(kConstant->getValue()->first);
    }
}

///Instruction_function
std::string Instruction_function::getName() const
{
    return "function";
}

void Instruction_function::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kName = KeywordName::parse(input._arguments[0], data);
    if (!kName.has_value())
    {
        throw codeg::ArgumentError(1, "name");
    }

    if ( data._functions.get(kName->getName()) != nullptr )
    {
        throw codeg::CompileError("bad function (function \""+kName->getName()+"\" already exist)");
    }

    if ( !data._actualFunctionName.empty() )
    {
        throw codeg::CompileError("function error (can't create a function in a function)");
    }
    if ( data._scopes.size() )
    {
        throw codeg::CompileError("function error (can't create a function in a scope)");
    }

    data._scopes.newScope(codeg::ScopeStats::SCOPE_FUNCTION, data._reader.getLineCount(), data._reader.getPath()); //New scope

    data._actualFunctionName = kName->getName();
    data._functions.push(kName->getName());

    data._jumps._jumpPoints.push_back({"%%E"+kName->getName(), data._code.getCursor()}); //Jump to the end of the function
    data._code.pushEmptyJump();

    if ( !data._jumps.addLabel({"%%"+kName->getName(), 0, data._code.getCursor()}) )
    {//Label to the start of the function
        throw codeg::CompileError("label error (label \"%%"+kName->getName()+"\" already exist)");
    }

    ConsoleInfo << "\tstart of the function \""<< kName->getName() <<"\" at address : " << data._code.getCursor() << std::endl;
}

///Instruction_if
std::string Instruction_if::getName() const
{
    return "if";
}

void Instruction_if::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kValue = KeywordValue::parse(input._arguments[0], data);

    if (!kValue.has_value())
    {
        throw codeg::ArgumentError(1, "value");
    }
    auto value = kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1});
    if (kValue->isVariable())
    {
        kValue->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
        data._code.pushEmptyVarAccess();
    }
    if (value.second != 1)
    {
        throw codeg::ByteSizeError(1, "1");
    }

    /*
    2 jump points must be created in order to work properly :
        - At the false condition, label named %%Fn where n is the scope
        - At the end of the condition, label named %%En where n is the scope
    if there is no "else" keyword, the label %%Fn will be the end of the condition
    */

    data._scopes.newScope(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE, data._reader.getLineCount(), data._reader.getPath()); //New scope

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopes.getScopeCount()), data._code.getCursor()});
    data._code.pushEmptyJumpAddress();

    data._code.push(codeg::OPCODE_IF | kValue->getReadableBusType().value());
    data._code.pushCheckDummy(value.first, kValue->getReadableBusType().value());
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_else
std::string Instruction_else::getName() const
{
    return "else";
}

void Instruction_else::compile([[maybe_unused]] const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
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
    auto kValue = KeywordValue::parse(input._arguments[0], data);

    if (!kValue.has_value())
    {
        throw codeg::ArgumentError(1, "value");
    }
    auto value = kValue->getValue().value_or(std::pair<uint32_t,std::size_t>{0,1});
    if (kValue->isVariable())
    {
        kValue->getVariableKeyword()->getVariable()->_link.push_back({data._code.getCursor()});
        data._code.pushEmptyVarAccess();
    }
    if (value.second != 1)
    {
        throw codeg::ByteSizeError(1, "1");
    }

    /*
    2 jump points must be created in order to work properly :
        - At the false condition, label named %%Fn where n is the scope
        - At the end of the condition, label named %%En where n is the scope
    if there is no "else" keyword, the label %%Fn will be the end of the condition
    */

    data._scopes.newScope(codeg::ScopeStats::SCOPE_CONDITIONAL_TRUE, data._reader.getLineCount(), data._reader.getPath()); //New scope

    data._jumps._jumpPoints.push_back({"%%F"+std::to_string(data._scopes.getScopeCount()), data._code.getCursor()});
    data._code.pushEmptyJumpAddress();

    data._code.push(codeg::OPCODE_IFNOT | kValue->getReadableBusType().value());
    data._code.pushCheckDummy(value.first, kValue->getReadableBusType().value());
    data._code.push(codeg::OPCODE_JMPSRC_CLK);
}

///Instruction_end
std::string Instruction_end::getName() const
{
    return "end";
}

void Instruction_end::compile([[maybe_unused]] const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
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
    auto kName = KeywordName::parse(input._arguments[0], data);
    if (!kName.has_value())
    {
        throw codeg::ArgumentError(1, "name");
    }

    if (input._arguments.size() == 4)
    {//call a function
        codeg::Function* func = data._functions.get(kName->getName());
        if ( func == nullptr )
        {
            throw codeg::CompileError("bad function (unknown function \""+kName->getName()+"\")");
        }
        if ( func->isDefinition() )
        {
            throw codeg::CompileError("bad function (can't call a definition with a return address)");
        }

        auto kVariableMSB = KeywordVariable::parse(input._arguments[1], data, true);
        auto kVariable = KeywordVariable::parse(input._arguments[2], data, true);
        auto kVariableLSB = KeywordVariable::parse(input._arguments[3], data, true);

        if ( !kVariableMSB.has_value() )
        {
            throw codeg::ArgumentError(2, "variable");
        }
        if ( !kVariable.has_value() )
        {
            throw codeg::ArgumentError(3, "variable");
        }
        if ( !kVariableLSB.has_value() )
        {
            throw codeg::ArgumentError(4, "variable");
        }

        //Prepare return address
        uint32_t returnAddress = data._code.getCursor() + 25;

        kVariableMSB->getVariable()->_link.push_back({data._code.getCursor()}); //MSB
        data._code.pushEmptyVarAccess();
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push((returnAddress&0x00FF0000)>>16);

        kVariable->getVariable()->_link.push_back({data._code.getCursor()}); //MSB
        data._code.pushEmptyVarAccess();
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push((returnAddress&0x0000FF00)>>8);

        kVariableLSB->getVariable()->_link.push_back({data._code.getCursor()}); //MSB
        data._code.pushEmptyVarAccess();
        data._code.push(codeg::OPCODE_RAMW | codeg::READABLE_SOURCE);
        data._code.push(returnAddress&0x000000FF);

        codeg::JumpPoint tmpPoint;
        tmpPoint._addressStatic = data._code.getCursor();
        tmpPoint._labelName = "%%"+kName->getName();

        if ( !data._jumps.addJumpPoint(tmpPoint) )
        {
            throw codeg::CompileError("bad label (unknown label \"%%"+kName->getName()+"\")");
        }
        data._code.pushEmptyJump();
    }
    else if ( input._arguments.size() == 1 )
    {//call a definition/function
        codeg::Function* func = data._functions.get(kName->getName());
        if ( func == nullptr )
        {
            throw codeg::CompileError("bad definition/function (unknown definition/function \""+kName->getName()+"\")");
        }
        if ( func->isDefinition() )
        {//definition
            data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_definition(func)) );
        }
        else
        {//function
            codeg::JumpPoint tmpPoint;
            tmpPoint._addressStatic = data._code.getCursor();
            tmpPoint._labelName = "%%"+kName->getName();

            if ( !data._jumps.addJumpPoint(tmpPoint) )
            {
                throw codeg::CompileError("bad label (unknown label \"%%"+kName->getName()+"\")");
            }
            data._code.pushEmptyJump();
        }
    }
}

///Instruction_clock
std::string Instruction_clock::getName() const
{
    return "clock";
}

void Instruction_clock::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kTarget = KeywordTarget::parse(input._arguments[0], data);

    if (!kTarget.has_value())
    {
        throw codeg::ArgumentError(1, "target");
    }

    uint8_t targetOpcode;
    switch (kTarget->getTarget())
    {
    case codeg::TargetType::TARGET_PERIPHERAL:
        targetOpcode = codeg::OPCODE_PERIPHERAL_CLK;
        break;

    default:
        throw codeg::CompileError("bad/unsupported target");
    }

    codeg::MemoryBigSize count = 1;
    if (input._arguments.size() == 2)
    {
        auto kConstant = KeywordConstant::parse(input._arguments[1], data);
        if (!kConstant.has_value())
        {
            throw codeg::ArgumentError(2, "constant");
        }
        if (kConstant->getValue()->first == 0)
        {
            ConsoleWarning << "you want 0 clock pulses, this instruction will be ignored" << std::endl;
        }
        count = kConstant->getValue()->first;
    }

    for (codeg::MemoryBigSize i=0; i<count; ++i)
    {
        data._code.push(targetOpcode | codeg::READABLE_DEFAULT);
        data._code.pushCheckDummy(0, codeg::READABLE_DEFAULT);
    }
}

///Instruction_pool
std::string Instruction_pool::getName() const
{
    return "pool";
}

void Instruction_pool::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kNamePoolName = KeywordName::parse(input._arguments[0], data);
    auto kConstantSize = KeywordConstant::parse(input._arguments[1], data);

    if (!kNamePoolName.has_value())
    {
        throw codeg::ArgumentError(1, "name");
    }
    if (!kConstantSize.has_value())
    {
        throw codeg::ArgumentError(2, "constant");
    }

    codeg::Address startAddress = 0;
    bool isDynamic = true;
    if (input._arguments.size() == 3)
    {
        auto kConstantStartAddress = KeywordConstant::parse(input._arguments[2], data);
        if ( !kConstantStartAddress.has_value() )
        {
            throw codeg::ArgumentError(3, "constant");
        }
        startAddress = kConstantStartAddress->getValue()->first;
        isDynamic = false;
    }

    if ( codeg::Pool* tmpPool = data._pools.getPool(kNamePoolName->getName()) )
    {//Pool already exist
        ConsoleWarning << "pool \"" << kNamePoolName->getName() << "\" already exist and will be replaced !" << std::endl;

        tmpPool->setStartAddressType(isDynamic ? codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC : codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC);
        tmpPool->setAddress(startAddress, kConstantSize->getValue()->first);
    }
    else
    {//Pool must be created
        codeg::Pool tmpNewPool(kNamePoolName->getName());

        tmpNewPool.setStartAddressType(isDynamic ? codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC : codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC);
        tmpNewPool.setAddress(startAddress, kConstantSize->getValue()->first);

        if ( !data._pools.addPool(tmpNewPool) )
        {
            throw codeg::FatalError("bad pool (pool \""+kNamePoolName->getName()+"\" already exist ?)");
        }
    }
}

///Instruction_offset
std::string Instruction_offset::getName() const
{
    return "offset";
}

void Instruction_offset::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kString = KeywordString::parse(input._arguments[0], data);
    auto kConstant = KeywordConstant::parse(input._arguments[1], data);

    if (!kString.has_value())
    {
        throw codeg::ArgumentError(1, "string");
    }
    if (!kConstant.has_value())
    {
        throw codeg::ArgumentError(2, "constant");
    }

    const std::string& string = kString.value().getString();

    if (string == "add")
    {
        codeg::MemoryBigSize count = kConstant->getValue()->first;

        if (count == 0)
        {
            ConsoleWarning << "offset of 0, no instruction will be added !" << std::endl;
            return;
        }
        if (data._code.getWriteDummy())
        {
            if (count%2 != 0)
            {
                throw codeg::CompileError("when dummy values must be placed, offset must be a multiple of 2 !");
            }

            for (codeg::MemoryBigSize i = 0; i < count/2; ++i)
            {
                data._code.push(codeg::OPCODE_STICK | codeg::READABLE_SOURCE);
                data._code.pushDummy();
            }
        }
        else
        {
            for (codeg::MemoryBigSize i = 0; i < count; ++i)
            {
                data._code.push(codeg::OPCODE_STICK | codeg::READABLE_DEFAULT);
                data._code.pushDummy();
            }
        }
    }
    else if (string == "reach")
    {
        codeg::MemoryBigSize reachAddress = kConstant->getValue()->first;
        codeg::MemoryBigSize actualAddress = data._code.getCursor();

        if (reachAddress < actualAddress)
        {
            throw codeg::CompileError("Can't reach the address "+std::to_string(reachAddress)+" as code address "+std::to_string(actualAddress)+" exceeds it !");
        }
        codeg::MemoryBigSize count = reachAddress - actualAddress;
        if (data._code.getWriteDummy())
        {
            if (count%2 != 0)
            {
                throw codeg::CompileError("when dummy values must be placed, offset must be a multiple of 2 !");
            }

            for (codeg::MemoryBigSize i = 0; i < count/2; ++i)
            {
                data._code.push(codeg::OPCODE_STICK | codeg::READABLE_SOURCE);
                data._code.pushDummy();
            }
        }
        else
        {
            for (codeg::MemoryBigSize i = 0; i < count; ++i)
            {
                data._code.push(codeg::OPCODE_STICK | codeg::READABLE_DEFAULT);
                data._code.pushDummy();
            }
        }
    }
    else
    {
        throw codeg::CompileError("bad argument (argument 1 [string] must be \"add\" or \"reach\")");
    }
}

///Instruction_import
std::string Instruction_import::getName() const
{
    return "import";
}

void Instruction_import::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    std::filesystem::path path = data._relativePath;
    path /= input._arguments[0];

    if ( !data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_file(path)) ) )
    {
        throw codeg::FatalError("can't open the file : \""+path.string()+"\"");
    }
}

///Instruction_definition
std::string Instruction_definition::getName() const
{
    return "definition";
}

void Instruction_definition::compile(const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    auto kName = KeywordName::parse(input._arguments[0], data);
    if ( !kName.has_value() )
    {
        throw codeg::ArgumentError(1, "name");
    }

    if ( data._functions.get(kName->getName()) != nullptr )
    {
        throw codeg::CompileError("bad definition (definition/function \""+kName->getName()+"\" already exist)");
    }

    if ( !data._scopes.empty() )
    {
        throw codeg::CompileError("definition error (can't create a definition in a scope)");
    }

    data._scopes.newScope(codeg::ScopeStats::SCOPE_DEFINITION, data._reader.getLineCount(), data._reader.getPath()); //New scope

    data._actualFunctionName = kName->getName();
    data._functions.push(kName->getName(), true);

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
void Instruction_enddef::compileDefinition([[maybe_unused]] const codeg::StringDecomposer& input, codeg::CompilerData& data)
{
    //Ending the definition
    data._writeLinesIntoDefinition = false;
    data._actualFunctionName.clear();

    data._scopes.pop();
}

}//end codeg
