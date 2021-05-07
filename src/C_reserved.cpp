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

#include "C_reserved.hpp"

namespace codeg
{

void ReservedList::clear()
{
    this->g_data.clear();
}

bool ReservedList::isReserved(const std::string& str)
{
    for (auto& val : this->g_data)
    {
        if (val == str)
        {
            return true;
        }
    }
    return false;
}
void ReservedList::push(const std::string& str)
{
    for (auto& val : this->g_data)
    {
        if (val == str)
        {
            return;
        }
    }
    this->g_data.push_front(str);
}
void ReservedList::push(std::string&& str)
{
    for (auto& val : this->g_data)
    {
        if (val == str)
        {
            return;
        }
    }
    this->g_data.push_front(std::move(str));
}

}//end codeg
