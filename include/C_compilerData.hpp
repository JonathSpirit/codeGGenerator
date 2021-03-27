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

#ifndef C_COMPILERDATA_H_INCLUDED
#define C_COMPILERDATA_H_INCLUDED

#include "C_fileReader.hpp"
#include "C_stringDecomposer.hpp"
#include "C_macro.hpp"
#include "C_variable.hpp"
#include "C_address.hpp"
#include "C_instruction.hpp"
#include <list>
#include <memory>
#include <stack>

namespace codeg
{

enum ScopeStats : uint32_t
{
    SCOPE_NORMAL,

    SCOPE_CONDITIONAL_TRUE,
    SCOPE_CONDITIONAL_FALSE
};

struct Scope
{
    uint32_t _id;
    unsigned int _startLine;
    std::string _startFile;
};

struct CodeData
{
    bool push(uint8_t d);
    void resize(uint32_t n);

    uint32_t _cursor = 0;
    uint32_t _capacity = 0;

    std::shared_ptr<uint8_t[]> _data;
};

struct CompilerData
{
    bool isReserved(const std::string& str);

    codeg::StringDecomposer _decomposer;

    std::list<codeg::Instruction*> _instructions;
    std::list<std::string> _reservedKeywords;

    codeg::PoolList _pools;
    std::string _defaultPool;

    codeg::MacroList _macros;

    codeg::JumpList _jumps;

    std::string _actualFunctionName;
    std::list<std::string> _functions;

    uint32_t _scopeCount=0;
    std::stack<codeg::Scope> _scope;
    std::stack<codeg::ScopeStats> _scopeStats;

    codeg::FileReader _reader;
    std::string _relativePath;

    codeg::CodeData _code;
};

}//end codeg

#endif // C_COMPILERDATA_H_INCLUDED
