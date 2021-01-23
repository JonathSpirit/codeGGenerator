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

#include "C_macro.hpp"
#include "main.hpp"

namespace codeg
{

bool MacroReplace(const codeg::MacroList& macroList, std::string& str)
{
    for (auto&& [first,second] : macroList)
    {
        if (str == first)
        {
            str = second;
            return true;
        }
    }
    return false;
}

bool MacroCheck(const codeg::MacroList& l, const std::string& r)
{
    return l.find(r) != l.cend();
}

}//end codeg
