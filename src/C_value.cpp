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

#include "C_value.h"
#include "main.h"

namespace codeg
{

size_t GetIntegerFromString2(const std::string& buffStr, uint32_t& buff)
{
    std::string str = buffStr;
    buff = 0;

    if ( !str.size() )
    {
        return 0;
    }

    if (str.size() > 2)
    {
        if ( str[0] == '0' )
        {
            if ( str[1] == 'x' )
            {//Hex
                uint32_t hexCount = str.size()-2;
                if ( hexCount>8 )
                {//Bad size
                    return 0;
                }

                std::transform(str.begin(), str.end(), str.begin(), ::toupper);

                uint32_t hexShift = 4*(hexCount-1);
                for (uint32_t i=2; i<str.size(); ++i)
                {
                    if ( (str[i] >= '0') && (str[i] <= '9') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'0') << hexShift;
                    }
                    else if ( (str[i] >= 'A') && (str[i] <= 'F') )
                    {
                        buff |= static_cast<uint32_t>(str[i]-'A') << hexShift;
                    }
                    else
                    {//Bad hex char
                        return 0;
                    }

                    hexShift -= 4;
                }

                return ((hexCount+1)/2);
            }

            if ( str[1] == 'b' )
            {//Binary
                uint32_t binCount = str.size()-2;
                if ( binCount>32 )
                {//Bad size
                    return 0;
                }

                uint32_t binShift = binCount-1;
                for (uint32_t i=2; i<str.size(); ++i)
                {
                    if ( str[i] == '1' )
                    {
                        buff |= static_cast<uint32_t>(1) << binShift;
                    }
                    else if ( str[i] != '0' )
                    {//Bad bin char
                        return 0;
                    }

                    binShift -= 1;
                }

                return ((binCount-1)/4) + 1;
            }
        }
    }

    //Decimal
    try
    {
        buff = std::stoul(str);

        if (buff < 0x10000)
        {
            if (buff < 0x100)
            {// 8 bit
                return 1;
            }
            else
            {// 16 bit
                return 2;
            }
        }
        else if (buff < 0x1000000L)
        {// 24 bit
            return 3;
        }
        else
        {// 32 bit
            return 4;
        }
    }
    catch (std::exception& e)
    {
        //Bad value
        return 0;
    }
}

bool GetIntegerFromString(std::string str, unsigned int& buff)
{
    if ( !str.size() )
    {
        return false;
    }

    buff = 0;

    if (str.size() > 2)
    {
        if ( str[0] == '0' )
        {
            if ( str[1] == 'x' )
            {//Hex
                unsigned int hexCount = str.size()-2;
                if ( (!hexCount) || (hexCount>8) )
                {
                    return false;
                }

                std::transform(str.begin(), str.end(), str.begin(), ::toupper);

                unsigned int hexShift = 4*(hexCount-1);
                for (unsigned int i=2; i<str.size(); ++i)
                {
                    if ( (str[i] >= '0') && (str[i] <= '9') )
                    {
                        buff |= static_cast<unsigned int>(str[i]-48) << hexShift;
                    }
                    else
                    {
                        if ( (str[i] >= 'A') && (str[i] <= 'F') )
                        {
                            buff |= static_cast<unsigned int>(str[i]-55) << hexShift;
                        }
                        else
                        {
                            return false;
                        }
                    }

                    hexShift -= 4;
                }
                return true;
            }
            if ( str[1] == 'b' )
            {//Binary
                unsigned int binCount = str.size()-2;
                if ( (!binCount) || (binCount>32) )
                {
                    return false;
                }

                unsigned int binShift = binCount-1;
                for (unsigned int i=2; i<str.size(); ++i)
                {
                    if ( str[i] == '1' )
                    {
                        buff |= static_cast<unsigned int>(1) << binShift;
                    }
                    else
                    {
                        if ( str[i] != '0' )
                        {
                            return false;
                        }
                    }

                    binShift -= 1;
                }
                return true;
            }
        }
    }

    //Decimal
    try
    {
        buff = std::stoul(str);
        return true;
    }
    catch (std::exception& e)
    {
        return false;
    }
}

codeg::Value ProcessValue(const std::string& strValue, const std::string& defaultPool, codeg::PoolList& poolList)
{
    #if 0
    ///[value]
    /**
    constant : 0, 245, 0x24, 0x40, 0b00100111 etc...
    read_value : _src, _bread1, _bread2, _result, etc...
    variable : $nameOfTheVariable
    **/
    codeg::Value result;
    if ( !GetIntegerFromString(strValue, result._value) )
    {//Is not a constant
        if ( poolList.isVariable(strValue) )
        {//Is a variable
            std::string varName = strValue.substr(1);
            result._poolName = defaultPool;

            std::string::size_type customPoolPos = varName.find_first_of(':');
            if (customPoolPos != std::string::npos)
            {
                result._poolName = varName.substr(customPoolPos+1);
                varName.erase(customPoolPos);
            }

            result._variable = poolList.getVar(result._poolName, varName);
            if ( result._variable )
            {//Valid variable
                result._readValue = RV_RAMVALUE;
                result._type = ValueType::VALUETYPE_VARIABLE;
                result._value = 0;
                return result;
            }
            else
            {//Not a valid variable
                throw LineError("variable ("+varName+") pool <"+result._poolName+"> doesn't exist !");
            }
        }
        else
        {//Is not a variable
            result._value = 0;
            result._type = ValueType::VALUETYPE_READ_VALUE;
            result._readValue = 0xFF;
            if ( strValue == "_src" )
            {
                result._readValue = RV_SRCVALUE;
            }
            if ( strValue == "_bread1" )
            {
                result._readValue = RV_BREAD1;
            }
            if ( strValue == "_bread2" )
            {
                result._readValue = RV_BREAD2;
            }
            if ( strValue == "_result" )
            {
                result._readValue = RV_OPRESULT;
            }
            if ( strValue == "_ram" )
            {
                result._readValue = RV_RAMVALUE;
            }
            if ( strValue == "_spi" )
            {
                result._readValue = RV_SPI;
            }
            if ( strValue == "_ext1" )
            {
                result._readValue = RV_EXT_1;
            }
            if ( strValue == "_ext2" )
            {
                result._readValue = RV_EXT_2;
            }

            if ( result._readValue != 0xFF )
            {
                return result;
            }
            throw LineError("invalid [value] ("+strValue+")");
        }
    }
    else
    {//Is a constant
        result._type = (result._value>0xFF) ? ValueType::VALUETYPE_BIGCONSTANT : ValueType::VALUETYPE_CONSTANT;
        result._readValue = RV_SRCVALUE;
        return result;
    }
    #endif
}

}//end codeg
