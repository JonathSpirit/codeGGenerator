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

#include "C_readableBus.hpp"

namespace codeg
{

namespace
{
const char* __stringReadableBusses[]=
{
    "SOURCE",

    "BREAD1",
    "BREAD2",

    "RESULT",
    "RAM",
    "SPI",

    "EXT1",
    "EXT2"
};
}//end

const char* ReadableBusToString(uint8_t opcode)
{
    opcode = (opcode&CODEG_READABLEBUSSES_MASK)>>5;
    return __stringReadableBusses[opcode];
}

}//end codeg
