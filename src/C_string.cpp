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

#include "C_string.hpp"
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

size_t SplitKeywords(const std::string& str, std::vector<std::string>& buff)
{
    bool isString = false;
    bool isChar = false;

    std::string splitString;
    splitString.reserve(str.size());

    for (unsigned int i=0; i<str.size(); ++i)
    {
        if (isString)
        {//In a string
            if (str[i] == '\"')
            {//End of the string or quotation in a char
                if (isChar)
                {
                    splitString += '\"';
                }
                else
                {
                    isString = false;
                }
            }
            else if (str[i] == '\'')
            {//Start of a char in a string
                isChar = !isChar;
                splitString += '\'';
            }
            else
            {
                splitString += str[i];
            }
        }
        else if (isChar)
        {//In a char
            if (str[i] == '\'')
            {//End of the char
                isChar = false;
                splitString += '\'';
            }
            else
            {
                splitString += str[i];
            }
        }
        else if (str[i] == '\'')
        {//Start of a char
            isChar = true;
            splitString += '\'';
        }
        else if (str[i] == '\"')
        {//Start of a string
            isString = true;
        }
        else if (str[i] == ' ')
        {//We must split
            buff.push_back(std::move(splitString));
            splitString.reserve(str.size()-i);
        }
        else
        {
            splitString += str[i];
        }
    }

    if (splitString.size() > 0)
    {//Push the remaining char
        buff.push_back(std::move(splitString));
    }
    return buff.size();
}

std::string ValueToHex(uint32_t val, bool removeExtraZero)
{
    char buff[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    std::string out = "0x";

    bool extraZeroFlag = true;

    uint32_t mask = 0xF0000000;
    for (unsigned int i=0; i<8; ++i)
    {
        char buffChar = buff[(val&mask) >> 4*(7-i)];
        if (extraZeroFlag && removeExtraZero)
        {
            if (buffChar != '0')
            {
                extraZeroFlag = false;
                out += buffChar;
            }
        }
        else
        {
            out += buffChar;
        }
        mask >>= 4;
    }
    if (extraZeroFlag)
    {
        out += '0';
    }

    return out;
}

std::string Uint8ToHex(uint8_t val)
{
    char buff[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    std::string out = "0x";

    out += buff[(val&0xF0) >> 4];
    out += buff[(val&0x0F)];

    return out;
}

std::string GetRelativePath(const std::string& filePath)
{
    std::string result = filePath;

    size_t pos = filePath.find_last_of('/');
    if (pos == std::string::npos)
    {
        return "";
    }
    result.erase(pos+1, std::string::npos);

    return result;
}

}//end codeg
