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

#ifndef C_STRINGDECOMPOSER_HPP_INCLUDED
#define C_STRINGDECOMPOSER_HPP_INCLUDED

#include "C_string.hpp"
#include <string>
#include <vector>

namespace codeg
{

enum StringDecomposerFlags : uint8_t
{
    FLAGS_EMPTY = 0x00,

    FLAG_NOCONTENT = 0x01,
    FLAG_IGNORE_CHAINING = 0x02
};

struct StringDecomposer
{
    void clear();
    void decompose(std::string str, uint8_t lastFlags=codeg::StringDecomposerFlags::FLAGS_EMPTY);

    uint8_t _flags = codeg::StringDecomposerFlags::FLAGS_EMPTY;
    std::string _brut;
    std::string _cleaned;

    std::string _instruction;
    std::vector<std::string> _arguments;
};

}//end codeg

#endif // C_STRINGDECOMPOSER_HPP_INCLUDED
