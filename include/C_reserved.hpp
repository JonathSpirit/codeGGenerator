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

#ifndef C_RESERVED_HPP_INCLUDED
#define C_RESERVED_HPP_INCLUDED

#include <forward_list>
#include <string>

namespace codeg
{

class ReservedList
{
public:
    using ReservedListType = std::forward_list<std::string>;

    ReservedList() = default;
    ~ReservedList() = default;

    void clear();

    bool isReserved(const std::string& str);
    void push(const std::string& str);
    void push(std::string&& str);

private:
    codeg::ReservedList::ReservedListType g_data;
};

}//end codeg

#endif // C_RESERVED_HPP_INCLUDED
