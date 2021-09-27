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

#include "C_address.hpp"
#include "C_compilerData.hpp"
#include "C_console.hpp"

namespace codeg
{

uint16_t Label::s_indexCount = 1;

void JumpList::resolve(codeg::CompilerData& data)
{
    //Name check
    for (auto&& vLabel : this->_labels)
    {
        if (vLabel._addressStatic >= data._code.getCursor())
        {//Address is out of code space
            codeg::ConsoleWarningWrite("Label \""+vLabel._name+"\" is out of code space with address : "+std::to_string(vLabel._addressStatic));
        }

        uint32_t jpCount = 0;

        for (auto&& vJumpPoint : this->_jumpPoints)
        {
            if (vLabel._name == vJumpPoint._labelName)
            {
                data._code[vJumpPoint._addressStatic+1] = (vLabel._addressStatic&0x00FF0000)>>16; //MSB
                data._code[vJumpPoint._addressStatic+3] = (vLabel._addressStatic&0x0000FF00)>>8;
                data._code[vJumpPoint._addressStatic+5] = (vLabel._addressStatic&0x000000FF); //LSB
                ++jpCount;
            }
        }

        codeg::ConsoleInfoWrite("\tLabel \""+vLabel._name+"\" with "+std::to_string(jpCount)+" jump points");
    }
}

bool JumpList::addLabel(const codeg::Label& d)
{
    //Name check
    for (auto&& value : this->_labels)
    {
        if (d._name == value._name)
        {
            return false;
        }
    }

    //Unique index check
    if (d._uniqueIndex == CODEG_NULL_UINDEX)
    {//Auto index
        bool valid = false;
        while (!valid)
        {
            if (codeg::Label::s_indexCount == 0)
            {
                ++codeg::Label::s_indexCount;
            }

            valid = true;
            for (auto&& value : this->_labels)
            {
                if (codeg::Label::s_indexCount == value._uniqueIndex)
                {
                    valid = false;
                    break;
                }
            }

            ++codeg::Label::s_indexCount;
        }
    }
    else
    {
        for (auto&& value : this->_labels)
        {
            if (d._uniqueIndex == value._uniqueIndex)
            {
                return false;
            }
        }
    }

    this->_labels.push_back(d);
    return true;
}
bool JumpList::addJumpPoint(const codeg::JumpPoint& d)
{
    ///TODO : add he jump point without checking label name
    //Jump to a label
    for (auto&& value : this->_labels)
    {
        if (d._labelName == value._name)
        {
            this->_jumpPoints.push_back(d);
            return true;
        }
    }

    return false;
}

std::list<codeg::Label>::iterator JumpList::getLabel(const std::string& name)
{
    for (std::list<codeg::Label>::iterator it=this->_labels.begin(); it!=this->_labels.end(); ++it)
    {
        if (name == (*it)._name)
        {
            return it;
        }
    }
    return this->_labels.end();
}

}//end codeg
