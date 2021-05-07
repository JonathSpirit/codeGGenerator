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

#include "C_compilerData.hpp"
#include "C_error.hpp"

namespace codeg
{

void CodeData::clear()
{
    this->g_data = nullptr;
    this->g_capacity = 0;
    this->g_cursor = 0;
}

void CodeData::push(uint8_t d)
{
    if (this->g_capacity == this->g_cursor)
    {
        throw codeg::FatalError("Code overflow, max is "+std::to_string(this->g_capacity));
    }

    this->g_data[this->g_cursor++] = d;
}
void CodeData::resize(uint32_t n)
{
    this->g_cursor = 0;
    this->g_capacity = n;

    this->g_data = std::shared_ptr<uint8_t[]>(new uint8_t[n]);
}

uint32_t CodeData::getCapacity() const
{
    return this->g_capacity;
}
uint32_t CodeData::getCursor() const
{
    return this->g_cursor;
}

void CodeData::set(uint32_t index, uint32_t value)
{
    if (index >= this->g_capacity)
    {
        throw codeg::FatalError("Index overflow, index is "+std::to_string(index)+" but capacity is "+std::to_string(this->g_capacity));
    }
    this->g_data.get()[index] = value;
}
uint8_t CodeData::get(uint32_t index) const
{
    if (index >= this->g_capacity)
    {
        throw codeg::FatalError("Index overflow, index is "+std::to_string(index)+" but capacity is "+std::to_string(this->g_capacity));
    }
    return this->g_data.get()[index];
}

uint8_t& CodeData::operator[](uint32_t index)
{
    if (index >= this->g_capacity)
    {
        throw codeg::FatalError("Index overflow, index is "+std::to_string(index)+" but capacity is "+std::to_string(this->g_capacity));
    }
    return this->g_data.get()[index];
}
const uint8_t& CodeData::operator[](uint32_t index) const
{
    if (index >= this->g_capacity)
    {
        throw codeg::FatalError("Index overflow, index is "+std::to_string(index)+" but capacity is "+std::to_string(this->g_capacity));
    }
    return this->g_data.get()[index];
}

uint8_t* CodeData::getData()
{
    return this->g_data.get();
}
std::shared_ptr<uint8_t[]>& CodeData::getSharedData()
{
    return this->g_data;
}

}//end codeg
