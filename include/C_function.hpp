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

#include "C_string.hpp"
#include <string>
#include <list>

namespace codeg
{

class Function
{
public:
    Function();
    Function(const std::string& name, bool definition=false);
    ~Function();

    void setName(const std::string& name);
    const std::string& getName() const;

    void setDefinitionType(bool definition);
    bool isDefinition() const;

    void clearLines();
    void addLine(const std::string& str);

    bool operator== (const std::string& l) const;

    std::list<std::string>::const_iterator getIteratorBegin() const;
    std::list<std::string>::const_iterator getIteratorEnd() const;

private:
    std::string g_name;

    bool g_isDefinition=false;
    std::list<std::string> g_definitionLines;
};

}//end codeg

#endif // C_FUNCTION_H_INCLUDED
