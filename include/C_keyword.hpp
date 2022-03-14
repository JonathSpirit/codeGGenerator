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

#ifndef C_KEYWORD_H_INCLUDED
#define C_KEYWORD_H_INCLUDED

#include <string>
#include <map>
#include <vector>
#include "C_target.hpp"
#include "C_compilerData.hpp"
#include "C_readableBus.hpp"

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

struct Keyword
{
    void clear();
    bool process(const std::string& str, const codeg::KeywordTypes& wantedType, codeg::CompilerData& data);

    codeg::KeywordTypes _type;

    std::string _str;

    uint32_t _value;
    size_t _valueSize;

    bool _valueIsVariable;
    bool _valueIsConst;

    codeg::ReadableBusses _valueBus;

    codeg::Variable* _variable;

    codeg::TargetType _target;
};

typedef std::vector<std::string> KeywordsList;
typedef std::map<std::string, std::string> CustomKeywordsList;

void ReplaceWithCustomKeywords(codeg::KeywordsList& keywords, codeg::CustomKeywordsList& customKeywords);

bool GetKeywordsFromString(std::string& str, codeg::KeywordsList& buffKeywords);

}//end codeg

#endif // C_KEYWORD_H_INCLUDED
