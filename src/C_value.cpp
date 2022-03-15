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

#include "C_value.hpp"
#include "C_error.hpp"

namespace codeg
{

std::size_t GetIntegerFromString(const std::string& buffStr, uint32_t& buff)
{
    std::string str = buffStr;
    buff = 0;

    //Return the size in byte

    if ( !str.size() )
    {
        return 0;
    }

    if (str.size() > 2)
    {
        if ( str[0] == '0' )
        {
            if ( str[1] == 'x' )
            {//Hex
                uint32_t hexCount = str.size()-2;
                if ( hexCount>8 )
                {//Bad size
                    throw codeg::SyntaxError("hex value must have a maximum size of 8, got "+std::to_string(hexCount));
                }

                uint32_t hexShift = 4*(hexCount-1);
                for (uint32_t i=2; i<str.size(); ++i)
                {
                    if ( (str[i] >= '0') && (str[i] <= '9') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'0') << hexShift;
                    }
                    else if ( (str[i] >= 'A') && (str[i] <= 'F') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'A'+10) << hexShift;
                    }
                    else if ( (str[i] >= 'a') && (str[i] <= 'f') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'a'+10) << hexShift;
                    }
                    else
                    {//Bad hex char
                        throw codeg::SyntaxError(std::string("bad char in hex value \'")+str[i]+"\'");
                    }

                    hexShift -= 4;
                }

                return ((hexCount+1)/2);
            }

            if ( str[1] == 'b' )
            {//Binary
                uint32_t binCount = str.size()-2;
                if ( binCount>32 )
                {//Bad size
                    throw codeg::SyntaxError("binary value must have a maximum size of 32, got "+std::to_string(binCount));
                }

                uint32_t binShift = binCount-1;
                for (uint32_t i=2; i<str.size(); ++i)
                {
                    if ( str[i] == '1' )
                    {
                        buff |= static_cast<uint32_t>(1) << binShift;
                    }
                    else if ( str[i] != '0' )
                    {//Bad bin char
                        throw codeg::SyntaxError(std::string("bad char in binary value \'")+str[i]+"\'");
                    }

                    binShift -= 1;
                }

                return ((binCount-1)/8) + 1;
            }
        }
    }

    if ( str[0] == '\'' )
    {//Char
        if (str.size() == 3)
        {
            if ( str[2] == '\'' )
            {
                buff = str[1];
                return 1;
            }
            else
            {
                throw codeg::SyntaxError(std::string("missing second quote (\') in char value, got \'")+str[2]+"\'");
            }
        }
        else
        {
            throw codeg::SyntaxError("char value must be a length of 3 characters, got "+std::to_string(str.size()));
        }
    }

    if ( (str[0] >= '0') && (str[0] <= '9') )
    {//Integer
        try
        {
            buff = std::stoul(str); ///TODO: replace stoul with custom function

            if (buff < 0x10000)
            {
                if (buff < 0x100)
                {// 8 bit
                    return 1;
                }
                else
                {// 16 bit
                    return 2;
                }
            }
            else if (buff < 0x1000000L)
            {// 24 bit
                return 3;
            }
            else
            {// 32 bit
                return 4;
            }
        }
        catch (std::exception& e)
        {//Bad integer
            throw codeg::SyntaxError("integer value must be a valid max 32bit integer composed with 0-9");
        }
    }

    //Not a constant value
    return 0;
}

}//end codeg
