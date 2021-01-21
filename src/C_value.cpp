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

#include "C_value.h"
#include "main.h"

namespace codeg
{

size_t GetIntegerFromString(const std::string& buffStr, uint32_t& buff)
{
    std::string str = buffStr;
    buff = 0;

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
                    return 0;
                }

                std::transform(str.begin(), str.end(), str.begin(), ::toupper);

                uint32_t hexShift = 4*(hexCount-1);
                for (uint32_t i=2; i<str.size(); ++i)
                {
                    if ( (str[i] >= '0') && (str[i] <= '9') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'0') << hexShift;
                    }
                    else if ( (str[i] >= 'A') && (str[i] <= 'F') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'A') << hexShift;
                    }
                    else
                    {//Bad hex char
                        return 0;
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
                    return 0;
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
                        return 0;
                    }

                    binShift -= 1;
                }

                return ((binCount-1)/4) + 1;
            }
        }
    }

    //Decimal
    try
    {
        buff = std::stoul(str);

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
    {
        //Bad value
        return 0;
    }
}

}//end codeg
