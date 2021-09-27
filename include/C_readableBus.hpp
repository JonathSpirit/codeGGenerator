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

#ifndef C_READABLEBUS_H_INCLUDED
#define C_READABLEBUS_H_INCLUDED

#include <cstdint>

#define CODEG_READABLEBUSSES_MASK 0xE0

namespace codeg
{

enum ReadableBusses : uint8_t
{
    READABLE_SOURCE = 0x00,

    READABLE_BREAD1 = 0x20,
    READABLE_BREAD2 = 0x40,

    READABLE_RESULT = 0x60,
    READABLE_RAM = 0x80,
    READABLE_SPI = 0xA0,

    READABLE_EXT1 = 0xC0,
    READABLE_EXT2 = 0xE0,

    READABLE_DEFAULT = READABLE_BREAD1
};

const char* ReadableBusToString(uint8_t opcode);

}//end codeg

#endif // C_READABLEBUS_H_INCLUDED
