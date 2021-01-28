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

std::string StrToHex(uint32_t val)
{
    char buff[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    std::string out = "0x";

    uint32_t mask = 0xF0000000;
    for (unsigned int i=0; i<8; ++i)
    {
        out += buff[(val&mask) >> 4*(7-i)];
        mask >>= 4;
    }

    return out;
}

}//end codeg
