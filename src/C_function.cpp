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

#include "C_function.hpp"

namespace codeg
{

///Function

Function::Function(const std::string& name, bool definition)
{
    this->g_name = name;
    this->g_isDefinition = definition;
}

void Function::setName(const std::string& name)
{
    this->g_name = name;
}
const std::string& Function::getName() const
{
    return this->g_name;
}

void Function::setDefinitionType(bool definition)
{
    this->g_isDefinition = definition;
}
bool Function::isDefinition() const
{
    return this->g_isDefinition;
}

void Function::clearLines()
{
    this->g_definitionLines.clear();
}
void Function::addLine(const std::string& str)
{
    this->g_definitionLines.push_back(str);
}

bool Function::operator== (const std::string& l) const
{
    return this->g_name == l;
}

codeg::Function::FunctionLinesType::const_iterator Function::getIteratorBegin() const
{
    return this->g_definitionLines.cbegin();
}
codeg::Function::FunctionLinesType::const_iterator Function::getIteratorEnd() const
{
    return this->g_definitionLines.cend();
}

///FunctionList

void FunctionList::clear()
{
    this->g_data.clear();
}

codeg::Function* FunctionList::push(const codeg::Function& newFunction)
{
    this->g_data.push_front(newFunction);
    return &this->g_data.front();
}
codeg::Function* FunctionList::push(const std::string& name, bool definition)
{
    this->g_data.emplace_front(name, definition);
    return &this->g_data.front();
}

codeg::Function* FunctionList::getLast()
{
    if ( this->g_data.empty() )
    {
        return nullptr;
    }
    return &this->g_data.front();
}
codeg::Function* FunctionList::get(const std::string& name)
{
    for (auto& value : this->g_data)
    {
        if (value.getName() == name)
        {
            return &value;
        }
    }
    return nullptr;
}

}//end codeg
