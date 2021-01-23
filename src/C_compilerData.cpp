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

#include "C_compilerData.h"
#include <algorithm>

namespace codeg
{

bool CompilerData::isReserved(const std::string& str)
{
    return std::find(this->_reservedKeywords.begin(), this->_reservedKeywords.end(), str) != this->_reservedKeywords.end();
}

bool CodeData::push(uint8_t d)
{
    if (this->_capacity == this->_cursor)
    {
        return false;
    }
    #ifdef __clang__
    reinterpret_cast<unsigned char*>(this->_data.get())[this->_cursor++] = d;
    #else
    this->_data[this->_cursor++] = d;
    #endif // __clang__

    return true;
}
void CodeData::resize(uint32_t n)
{
    this->_cursor = 0;
    this->_capacity = n;

    #ifdef __clang__
    this->_data = std::shared_ptr<uint8_t[]>( reinterpret_cast<std::nullptr_t>(new uint8_t[n]) );
    #else
    this->_data = std::shared_ptr<uint8_t[]>(new uint8_t[n]);
    #endif // __clang__
}

}//end codeg
