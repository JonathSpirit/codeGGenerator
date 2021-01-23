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

#include "C_variable.h"
#include "C_compilerData.h"
#include "C_console.h"
#include "main.h"

namespace codeg
{

///Pool

Pool::Pool(const std::string& name)
{
    this->g_name = name;
}
Pool::~Pool()
{

}

void Pool::clear()
{
    this->g_addressMaxSize = 0;
    this->g_startAddress = 0;
    this->g_startAddressType = codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC;
    this->g_variables.clear();
}
size_t Pool::getSize() const
{
    return this->g_variables.size();
}

void Pool::setName(const std::string& name)
{
    this->g_name = name;
}
const std::string& Pool::getName() const
{
    return this->g_name;
}

void Pool::setStartAddressType(const codeg::Pool::StartAddressTypes& type)
{
    this->g_startAddressType = type;
}
const codeg::Pool::StartAddressTypes& Pool::getStartAddressType() const
{
    return this->g_startAddressType;
}

bool Pool::setAddress(const codeg::MemoryAddress& start, const codeg::MemorySize& maxSize)
{
    if (maxSize == 0)
    {//Dynamic size
        this->g_addressMaxSize = 0;
        this->g_startAddress = start;
        return true;
    }

    if (this->g_startAddressType == codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC)
    {//Dynamic
        this->g_addressMaxSize = maxSize;
        this->g_startAddress = start;
        return true;
    }
    else
    {//Static
        if ( (static_cast<uint32_t>(start) + maxSize) > 0xFFFF )
        {//Out of range
            return false;
        }
        this->g_addressMaxSize = maxSize;
        this->g_startAddress = start;
        return true;
    }
}
codeg::MemoryAddress Pool::getStartAddress() const
{
    return this->g_startAddress;
}
codeg::MemorySize Pool::getMaxSize() const
{
    return this->g_addressMaxSize;
}
codeg::MemorySize Pool::getTotalSize() const
{
    if (this->g_addressMaxSize == 0)
    {
        return this->g_variables.size();
    }
    return this->g_addressMaxSize;
}

bool Pool::addVariable(const codeg::Variable& var)
{
    if (this->g_addressMaxSize != 0)
    {
        if (this->g_variables.size() >= this->g_addressMaxSize)
        {
            return false;
        }
    }

    for ( auto& value : this->g_variables )
    {
        if ( value._name == var._name )
        {
            return false;
        }
    }

    this->g_variables.push_back(var);
    return true;
}
codeg::Variable* Pool::getVariable(const std::string& name)
{
    for ( auto& value : this->g_variables )
    {
        if ( value._name == name )
        {
            return &value;
        }
    }
    return nullptr;
}
bool Pool::delVariable(const std::string& name)
{
    for ( std::list<codeg::Variable>::iterator it=this->g_variables.begin(); it!=this->g_variables.end(); ++it )
    {
        if ( (*it)._name == name )
        {
            this->g_variables.erase(it);
            return true;
        }
    }
    return false;
}

codeg::MemorySize Pool::resolveLinks(codeg::CompilerData& data, const codeg::MemoryAddress& startAddress)
{
    codeg::MemoryAddress offset = 0;
    for ( codeg::Variable& valVar : this->g_variables )
    {
        codeg::MemoryAddress varAdd = startAddress + offset;
        for ( codeg::Address& valTarget : valVar._link )
        {
            data._code._data[valTarget + 1] = varAdd >> 8;//Address MSB
            data._code._data[valTarget + 3] = varAdd & 0x00FF;//Address LSB
        }
        ++offset;
    }

    return this->g_variables.size();
}

///PoolList

PoolList::PoolList()
{

}
PoolList::~PoolList()
{

}

void PoolList::clear()
{
    this->g_pools.clear();
}
size_t PoolList::getSize() const
{
    return this->g_pools.size();
}

bool PoolList::addPool(codeg::Pool& newPool)
{
    for (auto& value : this->g_pools)
    {
        if (value.getName() == newPool.getName())
        {
            value = newPool;
            return true;
        }
    }
    this->g_pools.push_back(newPool);
    return true;
}
codeg::Pool* PoolList::getPool(const std::string& poolName)
{
    for (auto& value : this->g_pools)
    {
        if (value.getName() == poolName)
        {
            return &value;
        }
    }
    return nullptr;
}
bool PoolList::delPool(const std::string& poolName)
{
    for (std::list<codeg::Pool>::iterator it=this->g_pools.begin(); it!=this->g_pools.end(); ++it)
    {
        if ((*it).getName() == poolName)
        {
            this->g_pools.erase(it);
            return true;
        }
    }
    return false;
}

codeg::Variable* PoolList::getVariable(const std::string& varName, const std::string& poolName)
{
    for (auto& value : this->g_pools)
    {
        if (value.getName() == poolName)
        {
            return value.getVariable(varName);
        }
    }
    return nullptr;
}
codeg::Variable* PoolList::getVariableWithString(const std::string& str, const std::string& defaultPoolName)
{
    std::string varName;
    std::string poolName;

    if ( codeg::GetVariableString(str, defaultPoolName, varName, poolName) )
    {
        for (auto& value : this->g_pools)
        {
            if (value.getName() == poolName)
            {
                return value.getVariable(varName);
            }
        }
    }
    return nullptr;
}

codeg::MemorySize PoolList::resolve(codeg::CompilerData& data)
{
    codeg::MemorySize totalSize = 0;
    codeg::ConsoleInfoWrite( "Fixed start address only ..." );
    std::vector<std::list<codeg::Pool>::iterator> appliedPools;
    appliedPools.reserve(this->g_pools.size());

    for ( std::list<codeg::Pool>::iterator it = this->g_pools.begin(); it!=this->g_pools.end(); ++it )
    {
        if ( (*it).getStartAddressType() == codeg::Pool::StartAddressTypes::START_ADDRESS_STATIC )
        {
            codeg::ConsoleInfoWrite( "Working on pool \""+(*it).getName()+"\":" );
            codeg::ConsoleInfoWrite( "\tused size: "+std::to_string((*it).getSize()) );
            codeg::ConsoleInfoWrite( "\ttotal size: "+std::to_string((*it).getTotalSize()) );
            codeg::ConsoleInfoWrite( "\tstart address: "+std::to_string((*it).getStartAddress()) );
            if ( (*it).getTotalSize() == 0 )
            {//No variable and dynamic size
                codeg::ConsoleWarningWrite("\tNo variable in this pool with a dynamic size, the pool will be ignored !");
                continue;
            }

            codeg::ConsoleInfoWrite( "\tCheck if the pool can be applied ..." );
            //Check if the pool can be applied
            for ( unsigned int i=0; i<appliedPools.size(); ++i )
            {
                if ( ((*it).getStartAddress() >= (*appliedPools[i]).getStartAddress()) && ((*it).getStartAddress() < (*appliedPools[i]).getStartAddress()+(*appliedPools[i]).getTotalSize()) )
                {//Pool conflict
                    throw codeg::FatalError("\tPool conflict, "+(*it).getName()+" conflict with "+(*appliedPools[i]).getName()+" !");
                }
            }

            totalSize += (*it).resolveLinks(data, (*it).getStartAddress());
            appliedPools.push_back(it);
            codeg::ConsoleInfoWrite( "\tPool applied !" );
        }
    }

    codeg::ConsoleInfoWrite( "OK" );
    codeg::ConsoleInfoWrite( "Memory mapping ..." );

    std::vector<codeg::MemorySize> mapping;
    mapping.resize(65536, 1);
    for ( unsigned int i=0; i<appliedPools.size(); ++i )
    {
        for ( unsigned int a=0; a<(*appliedPools[i]).getTotalSize(); ++a )
        {//Removing memory location already used
            mapping[(*appliedPools[i]).getStartAddress() + a] = 0;
        }
    }

    codeg::ConsoleInfoWrite( "OK" );
    codeg::ConsoleInfoWrite( "Dynamic start address only ..." );

    for ( std::list<codeg::Pool>::iterator it = this->g_pools.begin(); it!=this->g_pools.end(); ++it )
    {
        if ( (*it).getStartAddressType() == codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC )
        {
            bool isApplied = false;
            codeg::ConsoleInfoWrite( "Working on pool \""+(*it).getName()+"\":" );
            codeg::ConsoleInfoWrite( "\tused size: "+std::to_string((*it).getSize()) );
            codeg::ConsoleInfoWrite( "\ttotal size: "+std::to_string((*it).getTotalSize()) );
            if ( (*it).getTotalSize() == 0 )
            {//No variable and dynamic size
                codeg::ConsoleWarningWrite("\tNo variable in this pool with a dynamic size, the pool will be ignored !");
                continue;
            }

            codeg::ConsoleInfoWrite( "\tCheck if the pool can be applied ..." );
            //Check if the pool can be applied

            codeg::MemoryAddress memoryStart = 0;
            codeg::MemoryBigSize memorySize = 0;
            for ( unsigned int i=0; i<mapping.size(); ++i )
            {//Finding a free memory location
                if ( mapping[i] == 1 )
                {
                    memoryStart = i;
                    memorySize = 0;

                    for ( unsigned int a=i; a<mapping.size(); ++a )
                    {//Getting the size
                        if ( mapping[a] == 1 )
                        {
                            ++memorySize;
                        }
                        else
                        {
                            break;
                        }
                    }

                    //Check the size
                    if ( (*it).getTotalSize() <= memorySize )
                    {//size is ok !
                        totalSize += (*it).resolveLinks(data, memoryStart);
                        appliedPools.push_back(it);
                        codeg::ConsoleInfoWrite( "\tPool applied at address "+std::to_string(memoryStart)+" !" );
                        for ( unsigned int a=0; a<(*it).getTotalSize(); ++a )
                        {//Removing memory location that will be used
                            mapping[memoryStart + a] = 0;
                        }
                        isApplied = true;
                        break;
                    }
                }
            }
            if (!isApplied)
            {
                throw codeg::FatalError("\tDynamic pool doesn't have place in memory, "+(*it).getName()+" with size "+std::to_string((*it).getTotalSize())+" !");
            }
        }
    }

    codeg::ConsoleInfoWrite( "OK" );

    return totalSize;
}

bool IsVariable(const std::string& str)
{
    if (str.size() > 1)
    {
        return str.front() == '$';
    }
    return false;
}
bool GetVariableString(const std::string& str, const std::string& defaultPoolName, std::string& buffName, std::string& buffPool)
{
    bool poolName = false;

    buffName.clear();
    buffPool.clear();
    buffName.reserve(str.size());
    buffPool.reserve(str.size());

    if (str.size() > 1)
    {
        if (str.front() == '$')
        {
            for (uint32_t i=1; i<str.size(); ++i)
            {
                if (poolName)
                {
                    buffPool.push_back(str[i]);
                }
                else
                {
                    if (str[i] != ':')
                    {
                        buffName.push_back(str[i]);
                    }
                    else
                    {
                        poolName = true;
                    }
                }
            }

            if (!poolName || buffPool.empty())
            {
                buffPool = defaultPoolName;
            }

            buffName.shrink_to_fit();
            buffPool.shrink_to_fit();
            return true;
        }
    }

    return false;
}

}//end codeg
