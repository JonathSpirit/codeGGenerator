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

#ifndef C_ADDRESS_H_INCLUDED
#define C_ADDRESS_H_INCLUDED

#include <string>
#include <list>

#define CODEG_NULL_UINDEX 0

namespace codeg
{

struct CompilerData;

typedef uint32_t Address;

struct Label
{
    std::string _name;
    uint16_t _uniqueIndex;
    codeg::Address _addressStatic;

    static uint16_t s_indexCount;
};

struct JumpPoint
{
    std::string _labelName;
    codeg::Address _addressStatic;

    bool _isApplied = false;
};

struct JumpList
{
    void resolve(codeg::CompilerData& data);

    bool addLabel(const codeg::Label& d);
    bool addJumpPoint(const codeg::JumpPoint& d);

    std::list<codeg::Label>::iterator getLabel(const std::string& name);

    std::list<codeg::Label> _labels;
    std::list<codeg::JumpPoint> _jumpPoints;
};

}//end codeg

#endif // C_ADDRESS_H_INCLUDED
