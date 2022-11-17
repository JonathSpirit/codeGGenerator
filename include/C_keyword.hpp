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

#ifndef C_KEYWORD_HPP_INCLUDED
#define C_KEYWORD_HPP_INCLUDED

#include <string>
#include <optional>
#include <variant>
#include "C_target.hpp"
#include "C_compilerData.hpp"
#include "C_readableBus.hpp"
#include "C_bus.hpp"

namespace codeg
{

enum KeywordTypes : uint8_t
{
    KEYWORD_STRING = 0,
    KEYWORD_NAME,

    KEYWORD_TARGET,
    KEYWORD_BUS,

    KEYWORD_VALUE,
    KEYWORD_VARIABLE,
    KEYWORD_CONSTANT,

    KEYWORD_INSTRUCTION
};

class KeywordBase
{
public:
    KeywordBase() = default;
    virtual ~KeywordBase() = default;

    [[nodiscard]] virtual codeg::KeywordTypes getType() const = 0;

    [[nodiscard]] virtual std::optional<codeg::ReadableBusses> getReadableBusType() const;
    [[nodiscard]] virtual std::optional<std::pair<uint32_t, std::size_t>> getValue() const;

    [[nodiscard]] const std::string& getString() const;
    void setString(std::string str);

protected:
    std::string _g_string;
};

class KeywordString : public KeywordBase
{
public:
    KeywordString() = default;
    ~KeywordString() override = default;

    static std::optional<KeywordString> parse(const std::string& str, codeg::CompilerData& data);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_STRING;}
};

class KeywordName : public KeywordBase
{
public:
    KeywordName() = default;
    ~KeywordName() override = default;

    static std::optional<KeywordName> parse(const std::string& str, codeg::CompilerData& data);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_NAME;}

    [[nodiscard]] const std::string& getName() const;
    void setName(std::string str);

private:
    std::string g_name;
};

class KeywordConstant : public KeywordBase
{
public:
    KeywordConstant() = default;
    ~KeywordConstant() override = default;

    static std::optional<KeywordConstant> parse(const std::string& str, codeg::CompilerData& data);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_CONSTANT;}

    [[nodiscard]] std::optional<codeg::ReadableBusses> getReadableBusType() const override;

    [[nodiscard]] std::optional<std::pair<uint32_t, std::size_t>> getValue() const override;

    void setValue(uint32_t value);

private:
    uint32_t g_value{0};
    std::size_t g_valueByteSize{1};
};

class KeywordVariable : public KeywordBase
{
public:
    KeywordVariable() = default;
    ~KeywordVariable() override = default;

    static std::optional<KeywordVariable> parse(const std::string& str, codeg::CompilerData& data, bool mustExist=false);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_VARIABLE;}

    [[nodiscard]] std::optional<codeg::ReadableBusses> getReadableBusType() const override;

    [[nodiscard]] const codeg::Variable* getVariable() const;
    [[nodiscard]] codeg::Variable* getVariable();

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] const std::string& getPoolName() const;

    void setVariableName(const std::string& str);
    void setVariablePoolName(const std::string& poolName);

    void setVariable(codeg::Variable* variable);

    [[nodiscard]] bool exist() const;

private:
    std::string g_name;
    std::string g_poolName;
    codeg::Variable* g_variable{nullptr};
};

class KeywordValue : public KeywordBase
{
public:
    KeywordValue() = default;
    ~KeywordValue() override = default;

    static std::optional<KeywordValue> parse(const std::string& str, codeg::CompilerData& data);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_VALUE;}

    [[nodiscard]] std::optional<codeg::ReadableBusses> getReadableBusType() const override;

    [[nodiscard]] std::optional<std::pair<uint32_t, std::size_t>> getValue() const override;

    [[nodiscard]] std::optional<codeg::KeywordVariable> getVariableKeyword() const;
    [[nodiscard]] std::optional<codeg::KeywordConstant> getConstantKeyword() const;

    [[nodiscard]] bool isVariable() const;
    [[nodiscard]] bool isConstant() const;

private:
    std::variant<codeg::KeywordVariable, codeg::KeywordConstant, codeg::ReadableBusses> g_underlyingType;
};

class KeywordBus : public KeywordBase
{
public:
    KeywordBus() = default;
    ~KeywordBus() override = default;

    static std::optional<KeywordBus> parse(const std::string& str, codeg::CompilerData& data);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_BUS;}

    [[nodiscard]] codeg::BusTypes getBus() const;
    void setBus(codeg::BusTypes bus);

private:
    codeg::BusTypes g_bus{codeg::BusTypes::BUS_NULL};
};

class KeywordTarget : public KeywordBase
{
public:
    KeywordTarget() = default;
    ~KeywordTarget() override = default;

    static std::optional<KeywordTarget> parse(const std::string& str, codeg::CompilerData& data);

    [[nodiscard]] codeg::KeywordTypes getType() const override {return codeg::KeywordTypes::KEYWORD_TARGET;}

    [[nodiscard]] codeg::TargetType getTarget() const;
    void setTarget(codeg::TargetType target);

private:
    codeg::TargetType g_target{codeg::TargetType::TARGET_NULL};
};

}//end codeg

#endif // C_KEYWORD_HPP_INCLUDED
