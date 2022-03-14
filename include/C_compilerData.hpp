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

#ifndef C_COMPILERDATA_H_INCLUDED
#define C_COMPILERDATA_H_INCLUDED

#include "C_reserved.hpp"
#include "C_function.hpp"
#include "C_fileReader.hpp"
#include "C_stringDecomposer.hpp"
#include "C_macro.hpp"
#include "C_variable.hpp"
#include "C_address.hpp"
#include "C_instruction.hpp"
#include "C_readableBus.hpp"
#include <memory>
#include <stack>

namespace codeg
{

enum ScopeStats
{
    SCOPE_FUNCTION,
    SCOPE_DEFINITION,

    SCOPE_CONDITIONAL_TRUE,
    SCOPE_CONDITIONAL_FALSE
};

struct Scope
{
    uint32_t _id;
    codeg::ScopeStats _stat;

    unsigned int _startLine;
    std::string _startFile;
};

class ScopeList
{
public:
    using ScopeListType = std::stack<codeg::Scope>;

    ScopeList() = default;
    ~ScopeList() = default;

    void clear();

    void newScope(codeg::ScopeStats stat, unsigned int startLine, const std::string& startFile);

    const codeg::Scope& top() const;
    codeg::Scope& top();

    void pop();

    bool empty() const;
    size_t size() const;
    uint32_t getScopeCount() const;

private:
    codeg::ScopeList::ScopeListType g_data;
    uint32_t g_scopeCount = 0;
};

class CodeData
{
public:
    CodeData() = default;
    ~CodeData() = default;

    void clear();

    void pushFixedJump(uint32_t address);
    void pushEmptyJump();
    void pushEmptyJumpAddress();
    void pushFixedVarAccess(uint16_t address);
    void pushEmptyVarAccess();
    void push(uint8_t d);
    void pushCheckDummy(uint8_t d, codeg::ReadableBusses rbus);
    void pushDummy();
    void resize(uint32_t n);

    uint32_t getCapacity() const;
    uint32_t getCursor() const;

    void set(uint32_t index, uint32_t value);
    uint8_t get(uint32_t index) const;

    uint8_t& operator[](uint32_t index);
    const uint8_t& operator[](uint32_t index) const;

    uint8_t* getData();
    std::shared_ptr<uint8_t[]>& getSharedData();

    void setWriteDummy(bool value);
    bool getWriteDummy() const;

private:
    uint32_t g_cursor = 0;
    uint32_t g_capacity = 0;

    bool g_writeDummy = false;

    std::shared_ptr<uint8_t[]> g_data;
};

struct CompilerData
{
    codeg::StringDecomposer _decomposer;

    codeg::InstructionList _instructions;
    codeg::ReservedList _reservedKeywords;

    codeg::PoolList _pools;
    std::string _defaultPool;

    codeg::MacroList _macros;

    codeg::JumpList _jumps;

    bool _writeLinesIntoDefinition=false;
    std::string _actualFunctionName;
    codeg::FunctionList _functions;

    codeg::ScopeList _scopes;

    codeg::FileReader _reader;
    std::string _relativePath;

    codeg::CodeData _code;
};

}//end codeg

#endif // C_COMPILERDATA_H_INCLUDED
