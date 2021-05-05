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

#include "C_keyword.hpp"
#include "C_bus.hpp"
#include "C_value.hpp"
#include "C_variable.hpp"

namespace codeg
{

void Keyword::clear()
{
    this->_type = codeg::KeywordTypes::KEYWORD_STRING;

    this->_str.clear();

    this->_value = 0;
    this->_valueSize = 0;

    this->_valueIsConst = false;
    this->_valueIsVariable = false;

    this->_variable = nullptr;

    this->_target = codeg::TargetType::TARGET_NULL;
}

bool Keyword::process(const std::string& str, const codeg::KeywordTypes& wantedType, codeg::CompilerData& data)
{
    this->clear();
    this->_str = str;

    ///String
    if (wantedType == codeg::KeywordTypes::KEYWORD_STRING)
    {
        this->_type = codeg::KeywordTypes::KEYWORD_STRING;
        return true;
    }

    ///Instruction
    for (auto&& value : data._instructions)
    {
        if (value->getName() == str)
        {
            this->_type = codeg::KeywordTypes::KEYWORD_INSTRUCTION;
            return this->_type == wantedType;
        }
    }

    ///Replacing with an existing macro
    codeg::MacroReplace(data._macros, this->_str);

    ///Target
    if ( (this->_str == "PERIPHERAL") || (this->_str == "P") )
    {
        this->_type = codeg::KeywordTypes::KEYWORD_TARGET;
        this->_target = codeg::TargetType::TARGET_PERIPHERAL;
        return this->_type == wantedType;
    }
    else if ( (this->_str == "OPERATION") || (this->_str == "OP") )
    {
        this->_type = codeg::KeywordTypes::KEYWORD_TARGET;
        this->_target = codeg::TargetType::TARGET_OPERATION;
        return this->_type == wantedType;
    }
    else if ( this->_str == "SPI" )
    {
        this->_type = codeg::KeywordTypes::KEYWORD_TARGET;
        this->_target = codeg::TargetType::TARGET_SPI;
        return this->_type == wantedType;
    }

    ///Value (constant)
    this->_valueSize = GetIntegerFromString(this->_str, this->_value);
    if (this->_valueSize)
    {
        if ( (wantedType==codeg::KeywordTypes::KEYWORD_BUS) ||
             (wantedType==codeg::KeywordTypes::KEYWORD_VALUE) )
        {
            this->_type = wantedType;
        }
        else
        {
            this->_type = codeg::KeywordTypes::KEYWORD_CONSTANT;
        }

        this->_valueBus = codeg::ReadableBusses::READABLE_SOURCE;
        this->_valueIsConst = true;
        return this->_type == wantedType;
    }

    ///ReadableBusses
    if (this->_str == "_src")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_SOURCE;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_bread1")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_BREAD1;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_bread2")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_BREAD2;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_result")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_RESULT;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_ram")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_RAM;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_spi")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_SPI;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_ext1")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_EXT1;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }
    else if (this->_str == "_ext2")
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VALUE;
        this->_valueBus = codeg::ReadableBusses::READABLE_EXT2;
        this->_valueSize = 1;
        this->_value = 0;
        return this->_type == wantedType;
    }

    ///Variable
    if ( (this->_variable = data._pools.getVariableWithString(this->_str, data._defaultPool)) )
    {
        this->_type = codeg::KeywordTypes::KEYWORD_VARIABLE;
        this->_valueBus = codeg::ReadableBusses::READABLE_RAM;
        this->_valueIsVariable = true;
        return this->_type == wantedType;
    }

    ///Name
    if (wantedType == codeg::KeywordTypes::KEYWORD_NAME)
    {
        this->_type = codeg::KeywordTypes::KEYWORD_NAME;
        return !data.isReserved(str);
    }

    return false;
}

void ReplaceWithCustomKeywords(codeg::KeywordsList& keywords, codeg::CustomKeywordsList& customKeywords)
{
    for (unsigned int i=0; i<keywords.size(); ++i)
    {
        for (codeg::CustomKeywordsList::iterator it=customKeywords.begin(); it!=customKeywords.end(); ++it)
        {
            if (keywords[i] == it->first)
            {
                keywords[i] = it->second;
                break;
            }
        }
    }
}

bool GetKeywordsFromString(std::string& str, codeg::KeywordsList& buffKeywords)
{
    buffKeywords.clear();

    std::string result;

    while ( str.size() )
    {
        result.clear();
        result.reserve( str.size() );

        //Remove unnecessary begin char
        for (unsigned int i=0; i<str.size(); ++i)
        {
            if ( (str[i] > 0x20) && (str[i] < 0x7F) )
            {//Only certain ASCII char
                str.erase(0, i);
                break;
            }
        }
        //Getting the keyword
        unsigned int ikey;
        for (ikey=0; ikey<str.size(); ++ikey)
        {
            if ( str[ikey] == '#' )
            {//Comment !
                break;
            }

            if ( (str[ikey] > 0x20) && (str[ikey] < 0x7F) )
            {//Only certain ASCII char
                result.push_back(str[ikey]);
            }
            else
            {
                break;
            }
        }
        str.erase(0, ikey);

        if ( result.size() )
        {
            buffKeywords.push_back(result);
        }
        else
        {
            break;
        }
    }

    return buffKeywords.size();
}

}//end codeg
