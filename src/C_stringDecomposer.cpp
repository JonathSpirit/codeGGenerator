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

#include "C_stringDecomposer.hpp"
#include <sstream>

namespace codeg
{

size_t Split(const std::string& str, std::vector<std::string>& buff, char delimiter)
{
   std::string buffStr;
   std::istringstream strStream(str);
   while (std::getline(strStream, buffStr, delimiter))
   {
      buff.push_back(buffStr);
   }
   return buff.size();
}

void StringDecomposer::clear()
{
    this->_flags = codeg::StringDecomposerFlags::FLAGS_EMPTY;
    this->_brut.clear();
    this->_cleaned.clear();
    this->_keywords.clear();
}
void StringDecomposer::decompose(const std::string& str, uint8_t lastFlags)
{
    char lastChar = ' ';
    bool ignoringSpace = true;
    bool ignoring = false;

    this->_brut = str;

    this->_flags = codeg::StringDecomposerFlags::FLAGS_EMPTY;
    if ( lastFlags & codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING )
    {
        this->_flags |= codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING;
        ignoring = true;
    }

    this->_cleaned.clear();
    this->_keywords.clear();

    this->_cleaned.reserve(str.size());

    for (uint32_t i=0; i<str.size(); ++i)
    {
        if ( (lastChar == '#') && (str[i] == '[') )
        {//Possible begin of multi-line comments
            this->_flags |= codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING;
            ignoring = true;
            //goto skip;
        }

        if ( str[i] == '#' )
        {//Comment
            if ( lastChar == ']' )
            {//Possible end of multi-line comments
                this->_flags &=~ codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING;
                ignoring = false;
                lastChar = '#';
                ignoringSpace = true;
                continue;
            }
            else
            {
                ignoring = true;
            }
        }

        if (ignoring)
        {
            lastChar = str[i];
            ignoringSpace = true;
            continue;
        }


        if ( str[i] == ' ' )
        {//Space
            if ( !ignoringSpace )
            {
                this->_cleaned.push_back(' ');
                ignoringSpace = true;
            }
        }
        else if ( (str[i] > 0x20) && (str[i] < 0x7F) )
        {//Only certain ASCII char
            this->_cleaned.push_back(str[i]);
            ignoringSpace = false;
        }

        lastChar = str[i];
    }
    if ( this->_cleaned.back() == ' ' )
    {
        this->_cleaned.pop_back();
    }

    this->_cleaned.shrink_to_fit();
    codeg::Split(this->_cleaned, this->_keywords, ' ');
}

}//end codeg
