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

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

#define OPC_BWRITE1_CLK 0x00
#define OPC_BWRITE2_CLK 0x01
#define OPC_BPCS_CLK 0x02
#define OPC_OPLEFT_CLK 0x03
#define OPC_OPRIGHT_CLK 0x04
#define OPC_OPC_CLK 0x05
#define OPC_PERIPHERAL_CLK 0x06
#define OPC_BJMPSRC1_CLK 0x07
#define OPC_BJMPSRC2_CLK 0x08
#define OPC_BJMPSRC3_CLK 0x09
#define OPC_JMPSRC_CLK 0x0A
#define OPC_BRAMADD1_CLK 0x0B
#define OPC_BRAMADD2_CLK 0x0C
#define OPC_SPI_CLK 0x0D
#define OPC_BSPI_CLK 0x0E
#define OPC_STICK 0x0F
#define OPC_IF 0x10
#define OPC_IFNOT 0x11
#define OPC_RAMW 0x12
#define OPC_LTICK 0x17

#define RV_SRCVALUE 0x00 //000X XXXX
#define RV_BREAD1 0x20   //001X XXXX
#define RV_BREAD2 0x40   //010X XXXX
#define RV_OPRESULT 0x60 //011X XXXX
#define RV_RAMVALUE 0x80 //100X XXXX
#define RV_SPI 0xA0      //101X XXXX
#define RV_EXT_1 0xC0    //110X XXXX
#define RV_EXT_2 0xE0    //111X XXXX

//std::vector<std::string> Explode (const std::string& str, char arg);
extern std::vector<std::string> _global_bannedKeywords;

class LineError : public std::exception
{
public:
    LineError(const std::string& err);

    virtual const char* what() const noexcept;

private:
    std::string g_errstr;
};

#endif // MAIN_H_INCLUDED
