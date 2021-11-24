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

#ifndef C_BUS_H_INCLUDED
#define C_BUS_H_INCLUDED

#include <cstdint>

namespace codeg
{

enum BusTypes : uint8_t
{
    BUS_NULL = 0,

    BUS_WRITEABLE_1 = 1,
    BUS_WRITEABLE_2 = 2,

    BUS_SPICFG = 3,
    BUS_SPI    = 4,

    BUS_OPLEFT = 5,
    BUS_OPRIGHT = 6,

    BUS_UNKNOWN = 0xFF
};

}//end codeg

#endif // C_BUS_H_INCLUDED
