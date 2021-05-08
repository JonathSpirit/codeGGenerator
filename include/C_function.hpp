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

#ifndef C_FUNCTION_H_INCLUDED
#define C_FUNCTION_H_INCLUDED

#include <string>
#include <list>
#include <forward_list>

namespace codeg
{

class Function
{
public:
    using FunctionLinesType = std::list<std::string>;

    Function() = default;
    Function(const std::string& name, bool definition=false);
    ~Function() = default;

    void setName(const std::string& name);
    const std::string& getName() const;

    void setDefinitionType(bool definition);
    bool isDefinition() const;

    void clearLines();
    void addLine(const std::string& str);

    bool operator== (const std::string& l) const;

    codeg::Function::FunctionLinesType::const_iterator getIteratorBegin() const;
    codeg::Function::FunctionLinesType::const_iterator getIteratorEnd() const;

private:
    std::string g_name;

    bool g_isDefinition=false;
    codeg::Function::FunctionLinesType g_definitionLines;
};

class FunctionList
{
public:
    using FunctionListType = std::forward_list<codeg::Function>;

    FunctionList() = default;
    ~FunctionList() = default;

    void clear();

    codeg::Function* push(const codeg::Function& newFunction);
    codeg::Function* push(const std::string& name, bool definition=false);

    codeg::Function* getLast();
    codeg::Function* get(const std::string& name);

private:
    codeg::FunctionList::FunctionListType g_data;
};

}//end codeg

#endif // C_FUNCTION_H_INCLUDED
