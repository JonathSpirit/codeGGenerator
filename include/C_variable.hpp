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

#ifndef C_VARIABLE_HPP_INCLUDED
#define C_VARIABLE_HPP_INCLUDED

#include "C_address.hpp"
#include <list>

namespace codeg
{

struct CompilerData;

using MemoryAddress = uint16_t;
using MemorySize = uint16_t;
using MemoryBigSize = uint32_t;

struct VariableLink
{
    codeg::Address _address{0};
    codeg::MemorySize _offset{0};
};

struct Variable
{
    std::string _name;
    codeg::MemorySize _size;
    std::list<codeg::VariableLink> _link;
};

class Pool
{
public:
    enum StartAddressTypes
    {
        START_ADDRESS_DYNAMIC,
        START_ADDRESS_STATIC
    };

public:
    explicit Pool(std::string name);
    ~Pool() = default;

    void clear();
    [[nodiscard]] std::size_t getVariableSize() const;

    void setName(const std::string& name);
    [[nodiscard]] const std::string& getName() const;

    void setStartAddressType(const codeg::Pool::StartAddressTypes& type);
    [[nodiscard]] const codeg::Pool::StartAddressTypes& getStartAddressType() const;

    bool setAddress(const codeg::MemoryAddress& start, const codeg::MemorySize& maxSize);
    [[nodiscard]] codeg::MemoryAddress getStartAddress() const;
    [[nodiscard]] codeg::MemorySize getMaxSize() const;
    [[nodiscard]] codeg::MemoryBigSize getMemorySize() const;
    [[nodiscard]] codeg::MemorySize getTotalSize() const;

    bool addVariable(const codeg::Variable& var);
    bool addVariable(const std::string& name, codeg::MemorySize size);
    codeg::Variable* getVariable(const std::string& name);
    bool delVariable(const std::string& name);

    codeg::MemorySize resolveLinks(codeg::CompilerData& data, const codeg::MemoryAddress& startAddress);

    struct PoolLink
    {
        codeg::Address _address;
        codeg::MemorySize _offset;
    };
    std::list<codeg::Pool::PoolLink> _link;

private:
    std::string g_name;

    codeg::Pool::StartAddressTypes g_startAddressType{codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC};
    codeg::MemoryAddress g_startAddress{0};
    codeg::MemorySize g_addressMaxSize{0};

    std::list<codeg::Variable> g_variables;
};

class PoolList
{
public:
    PoolList() = default;
    ~PoolList() = default;

    void clear();
    [[nodiscard]] std::size_t getSize() const;

    bool addPool(codeg::Pool& newPool);
    codeg::Pool* getPool(const std::string& poolName);
    bool delPool(const std::string& poolName);

    codeg::Variable* getVariable(const std::string& varName, const std::string& poolName);
    codeg::Variable* getVariableWithString(const std::string& str, const std::string& defaultPoolName);

    codeg::MemoryBigSize resolve(codeg::CompilerData& data);

private:
    std::list<codeg::Pool> g_pools;
};

bool IsVariable(const std::string& str);
bool GetVariableString(const std::string& str, const std::string& defaultPoolName, std::string& buffName, std::string& buffPool);

}//end codeg

#endif // C_VARIABLE_HPP_INCLUDED
