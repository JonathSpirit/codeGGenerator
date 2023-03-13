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

#include "C_macro.hpp"
#include "C_error.hpp"

namespace codeg
{

//MacroList

void MacroList::clear()
{
    this->g_data.clear();
}

bool MacroList::replace(std::string& str) const
{
    std::size_t replacementCount = 0;
    do
    {
        auto it = this->g_data.find(str);
        if (it == this->g_data.end())
        {
            return replacementCount > 0;
        }
        str = it->second;
    }
    while (++replacementCount < CODEG_MAX_REPLACEMENT_COUNT);

    throw codeg::FatalError("macro: maximum replacement count reached, there is a loop somewhere !");
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
    return this->g_data.find(key) != this->g_data.end();
}

//InlinedStaticMacroList

void InlinedStaticMacroList::clear()
{
    this->g_data.clear();
}

std::optional<std::string> InlinedStaticMacroList::getReplacement(const std::string& str) const
{
    auto it = this->g_data.find(str);
    if (it != this->g_data.end())
    {
        return it->second();
    }
    return std::nullopt;
}

void InlinedStaticMacroList::set(const std::string& key, std::function<std::optional<std::string>()> func)
{
    this->g_data[key] = std::move(func);
}

bool InlinedStaticMacroList::remove(const std::string& key)
{
    return this->g_data.erase(key) > 0;
}
bool InlinedStaticMacroList::check(const std::string& key) const
{
    return this->g_data.find(key) != this->g_data.end();
}

}//end codeg
