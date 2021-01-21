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

#ifndef C_VARIABLE_H_INCLUDED
#define C_VARIABLE_H_INCLUDED

#include "main.h"
#include "C_address.h"

namespace codeg
{

class CompilerData;

typedef uint16_t MemoryAddress;
typedef uint16_t MemorySize;

struct Variable
{
    std::string _name;
    std::list<codeg::Address> _link;
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
    Pool(const std::string& name);
    ~Pool();

    void clear();
    size_t getSize() const;

    void setName(const std::string& name);
    const std::string& getName() const;

    void setStartAddressType(const codeg::Pool::StartAddressTypes& type);
    const codeg::Pool::StartAddressTypes& getStartAddressType() const;

    bool setAddress(const codeg::MemoryAddress& start, const codeg::MemorySize& maxSize);

    bool addVariable(const codeg::Variable& var);
    codeg::Variable* getVariable(const std::string& name);
    bool delVariable(const std::string& name);

    codeg::MemorySize resolveLinks(codeg::CompilerData& data);

private:
    std::string g_name;

    codeg::Pool::StartAddressTypes g_startAddressType;
    codeg::MemoryAddress g_startAddress;
    codeg::MemorySize g_addressMaxSize;

    std::list<codeg::Variable> g_variables;
};

class PoolList
{
public:
    PoolList();
    ~PoolList();

    void clear();
    size_t getSize() const;

    bool addPool(codeg::Pool& newPool);
    codeg::Pool* getPool(const std::string& poolName);
    bool delPool(const std::string& poolName);

    codeg::Variable* getVariable(const std::string& varName, const std::string& poolName);
    codeg::Variable* getVariableWithString(const std::string& str, const std::string& defaultPoolName);

    codeg::MemorySize resolve(codeg::CompilerData& data);

private:
    std::list<codeg::Pool> g_pools;
};

bool IsVariable(const std::string& str);
bool GetVariableString(const std::string& str, const std::string& defaultPoolName, std::string& buffName, std::string& buffPool);

bool ReadOnRam(const std::string& poolName, const std::string& varName, codeg::PoolList& poolList, std::vector<char>& codeData, unsigned int& cursor);
void ReadOnRam(const unsigned short address, std::vector<char>& codeData, unsigned int& cursor);

}//end codeg

#endif // C_VARIABLE_H_INCLUDED
