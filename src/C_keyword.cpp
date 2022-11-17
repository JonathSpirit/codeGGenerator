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

#include "C_keyword.hpp"
#include "C_bus.hpp"
#include "C_value.hpp"
#include "C_variable.hpp"

namespace codeg
{

//KeywordBase

std::optional<codeg::ReadableBusses> KeywordBase::getReadableBusType() const
{
    return std::nullopt;
}
std::optional<std::pair<uint32_t, std::size_t>> KeywordBase::getValue() const
{
    return std::nullopt;
}

const std::string& KeywordBase::getString() const
{
    return this->_g_string;
}
void KeywordBase::setString(std::string str)
{
    this->_g_string = std::move(str);
}

//KeywordString

std::optional<KeywordString> KeywordString::parse(const std::string& str, [[maybe_unused]] codeg::CompilerData& data)
{
    KeywordString output;
    output.setString(str);
    return output;
}

//KeywordName

std::optional<KeywordName> KeywordName::parse(const std::string& str, codeg::CompilerData& data)
{
    KeywordName output;
    output.setString(str);

    if ( data._instructions.get(str) != nullptr )
    {
        return std::nullopt;
    }

    //Replacing with an existing macro
    data._macros.replace(output._g_string);

    if (data._reservedKeywords.isReserved(output._g_string))
    {
        return std::nullopt;
    }

    //Target
    if (output._g_string == "PERIPHERAL" || output._g_string == "P" ||
        output._g_string == "OPERATION" || output._g_string == "OP")
    {
        return std::nullopt;
    }

    if (KeywordValue::parse(str, data).has_value())
    {
        return std::nullopt;
    }
    if (KeywordVariable::parse(str, data, false).has_value())
    {
        return std::nullopt;
    }

    output.setName(output._g_string);
    return output;
}

const std::string& KeywordName::getName() const
{
    return this->g_name;
}
void KeywordName::setName(std::string str)
{
    this->g_name = std::move(str);
}

//KeywordConstant

std::optional<KeywordConstant> KeywordConstant::parse(const std::string& str, codeg::CompilerData& data)
{
    KeywordConstant output;
    output.setString(str);

    //Replacing with an existing macro
    data._macros.replace(output._g_string);

    output.g_valueByteSize = GetIntegerFromString(output._g_string, output.g_value);
    if (output.g_valueByteSize != 0)
    {
        return output;
    }
    return std::nullopt;
}

std::optional<codeg::ReadableBusses> KeywordConstant::getReadableBusType() const
{
    return codeg::ReadableBusses::READABLE_SOURCE;
}

std::optional<std::pair<uint32_t, std::size_t>> KeywordConstant::getValue() const
{
    return std::pair<uint32_t, std::size_t>{this->g_value, this->g_valueByteSize};
}

void KeywordConstant::setValue(uint32_t value)
{
    this->g_value = value;

    if (value < 0x10000L)
    {
        if (value < 0x100L)
        {// 8 bit
            this->g_valueByteSize = 1;
        }
        else
        {// 16 bit
            this->g_valueByteSize = 2;
        }
    }
    else if (value < 0x1000000L)
    {// 24 bit
        this->g_valueByteSize = 3;
    }
    else
    {// 32 bit
        this->g_valueByteSize = 4;
    }
}

//KeywordVariable

std::optional<KeywordVariable> KeywordVariable::parse(const std::string& str, codeg::CompilerData& data, bool mustExist)
{
    KeywordVariable output;
    output.setString(str);

    if ( data._instructions.get(str) != nullptr )
    {
        return std::nullopt;
    }

    //Replacing with an existing macro
    data._macros.replace(output._g_string);

    if ( ParseVariableFromString(output._g_string, data._defaultPool, output.g_name, output.g_poolName) )
    {
        output.g_variable = data._pools.getVariable(output.g_name, output.g_poolName);

        if (mustExist && output.g_variable == nullptr)
        {
            return std::nullopt;
        }
        return output;
    }
    return std::nullopt;
}

std::optional<codeg::ReadableBusses> KeywordVariable::getReadableBusType() const
{
    return codeg::ReadableBusses::READABLE_RAM;
}

const codeg::Variable* KeywordVariable::getVariable() const
{
    return this->g_variable;
}
codeg::Variable* KeywordVariable::getVariable()
{
    return this->g_variable;
}

const std::string& KeywordVariable::getName() const
{
    return this->g_name;
}
const std::string& KeywordVariable::getPoolName() const
{
    return this->g_poolName;
}

void KeywordVariable::setVariableName(const std::string& str)
{
    this->g_name = str;
}
void KeywordVariable::setVariablePoolName(const std::string& poolName)
{
    this->g_poolName = poolName;
}

void KeywordVariable::setVariable(codeg::Variable* variable)
{
    this->g_variable = variable;
}

bool KeywordVariable::exist() const
{
    return this->g_variable != nullptr;
}

//KeywordValue

std::optional<KeywordValue> KeywordValue::parse(const std::string& str, codeg::CompilerData& data)
{
    KeywordValue output;
    output.setString(str);

    if ( data._instructions.get(str) != nullptr )
    {
        return std::nullopt;
    }

    //Replacing with an existing macro
    data._macros.replace(output._g_string);

    //Check if this is a constant
    auto kConstant = KeywordConstant::parse(str, data);
    if (kConstant.has_value())
    {
        output.g_underlyingType = kConstant.value();
        return output;
    }

    //Check if this is a ReadableBus
    if (output._g_string == "_src")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_SOURCE;
        return output;
    }
    else if (output._g_string == "_bread1")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_BREAD1;
        return output;
    }
    else if (output._g_string == "_bread2")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_BREAD2;
        return output;
    }
    else if (output._g_string == "_result")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_RESULT;
        return output;
    }
    else if (output._g_string == "_ram")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_RAM;
        return output;
    }
    else if (output._g_string == "_spi")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_SPI;
        return output;
    }
    else if (output._g_string == "_ext1")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_EXT1;
        return output;
    }
    else if (output._g_string == "_ext2")
    {
        output.g_underlyingType = codeg::ReadableBusses::READABLE_EXT2;
        return output;
    }

    //Variable
    auto kVariable = KeywordVariable::parse(str, data, true);
    if (kVariable.has_value())
    {
        output.g_underlyingType = kVariable.value();
        return output;
    }

    return std::nullopt;
}

std::optional<codeg::ReadableBusses> KeywordValue::getReadableBusType() const
{
    if ( std::holds_alternative<codeg::ReadableBusses>(this->g_underlyingType) )
    {
        return std::get<codeg::ReadableBusses>(this->g_underlyingType);
    }
    else if ( std::holds_alternative<codeg::KeywordVariable>(this->g_underlyingType) )
    {
        return std::get<codeg::KeywordVariable>(this->g_underlyingType).getReadableBusType();
    }
    else if ( std::holds_alternative<codeg::KeywordConstant>(this->g_underlyingType) )
    {
        return std::get<codeg::KeywordConstant>(this->g_underlyingType).getReadableBusType();
    }
    return std::nullopt;
}

std::optional<std::pair<uint32_t, std::size_t>> KeywordValue::getValue() const
{
    if ( std::holds_alternative<codeg::KeywordConstant>(this->g_underlyingType) )
    {
        return std::get<codeg::KeywordConstant>(this->g_underlyingType).getValue();
    }
    return std::nullopt;
}

std::optional<codeg::KeywordVariable> KeywordValue::getVariableKeyword() const
{
    if ( std::holds_alternative<codeg::KeywordVariable>(this->g_underlyingType) )
    {
        return std::get<codeg::KeywordVariable>(this->g_underlyingType);
    }
    return std::nullopt;
}
std::optional<codeg::KeywordConstant> KeywordValue::getConstantKeyword() const
{
    if ( std::holds_alternative<codeg::KeywordConstant>(this->g_underlyingType) )
    {
        return std::get<codeg::KeywordConstant>(this->g_underlyingType);
    }
    return std::nullopt;
}

bool KeywordValue::isVariable() const
{
    return std::holds_alternative<codeg::KeywordVariable>(this->g_underlyingType);
}
bool KeywordValue::isConstant() const
{
    return std::holds_alternative<codeg::KeywordConstant>(this->g_underlyingType);
}

//KeywordBus

std::optional<KeywordBus> KeywordBus::parse(const std::string& str, codeg::CompilerData& data)
{
    KeywordBus output;
    output.setString(str);

    //Replacing with an existing macro
    data._macros.replace(output._g_string);

    uint32_t value;
    auto byteSize = GetIntegerFromString(output._g_string, value);
    if (byteSize != 0)
    {
        switch (static_cast<codeg::BusTypes>(value))
        {
        case BUS_WRITEABLE_1:
        case BUS_WRITEABLE_2:
        case BUS_SPICFG:
        case BUS_SPI:
        case BUS_OPLEFT:
        case BUS_OPRIGHT:
            output.setBus(static_cast<codeg::BusTypes>(value));
            return output;
        default:
            break;
        }
    }
    return std::nullopt;
}

codeg::BusTypes KeywordBus::getBus() const
{
    return this->g_bus;
}
void KeywordBus::setBus(codeg::BusTypes bus)
{
    this->g_bus = bus;
}

//KeywordTarget

std::optional<KeywordTarget> KeywordTarget::parse(const std::string& str, codeg::CompilerData& data)
{
    KeywordTarget output;
    output.setString(str);

    if ( data._instructions.get(str) != nullptr )
    {
        return std::nullopt;
    }

    //Replacing with an existing macro
    data._macros.replace(output._g_string);

    if (output._g_string == "PERIPHERAL" || output._g_string == "P")
    {
        output.setTarget(codeg::TargetType::TARGET_PERIPHERAL);
        return output;
    }
    else if (output._g_string == "OPERATION" || output._g_string == "OP")
    {
        output.setTarget(codeg::TargetType::TARGET_OPERATION);
        return output;
    }

    return std::nullopt;
}

[[nodiscard]] codeg::TargetType KeywordTarget::getTarget() const
{
    return this->g_target;
}
void KeywordTarget::setTarget(codeg::TargetType target)
{
    this->g_target = target;
}

}//end codeg
