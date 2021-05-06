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

namespace codeg
{

void MacroList::clear()
{
    this->g_data.clear();
}

bool MacroList::replace(std::string& str) const
{
    for (auto& [key,value] : this->g_data)
    {
        if (str == key)
        {
            str = value;
            return true;
        }
    }
    return false;
}

void MacroList::set(const std::string& key, const std::string& str)
{
    this->g_data[key] = str;
}

bool MacroList::remove(const std::string& key)
{
    return this->g_data.erase(key) > 0;
}
bool MacroList::check(const std::string& key) const
{
    return this->g_data.find(key) != this->g_data.cend();
}

}//end codeg
