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

#ifndef C_VALUE_H_INCLUDED
#define C_VALUE_H_INCLUDED

#include "C_variable.h"

namespace codeg
{

enum ValueType
{
    VALUETYPE_BIGCONSTANT,
    VALUETYPE_CONSTANT,
    VALUETYPE_READ_VALUE,
    VALUETYPE_VARIABLE
};
struct Value
{
    ValueType _type;

    unsigned int _value;
    unsigned char _readValue;

    codeg::Variable* _variable;
    std::string _poolName;
};

size_t GetIntegerFromString2(const std::string& buffStr, uint32_t& buff);
bool GetIntegerFromString(std::string str, unsigned int& buff);

codeg::Value ProcessValue(const std::string& strValue, const std::string& defaultPool, codeg::PoolList& poolList);

}//end codeg

#endif // C_VALUE_H_INCLUDED
