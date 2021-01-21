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

codeg::MemorySize Pool::resolveLinks(codeg::CompilerData& data)
{
    codeg::MemoryAddress offset = 0;
    for ( auto& valVar : this->g_variables )
    {
        codeg::MemoryAddress varAdd = this->g_startAddress + offset;
        for ( auto& valTarget : valVar._link )
        {
            data._code._data.get()[valTarget + 1] = varAdd >> 8;//Address MSB
            data._code._data.get()[valTarget + 3] = varAdd & 0x00FF;//Address LSB
        }
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

}



/**
cout << "Resolving address pool ..." << endl;
    cout << "first pass, fixed address" << endl;
    cout << "\tsize : " << codePoolList._data.size() << endl;
    std::list<codeg::Pool> dynamicPool;
    for ( std::list<codeg::Pool>::iterator itPool=codePoolList._data.begin(); itPool!=codePoolList._data.end(); ++itPool )
    {
        if ( (*itPool)._dynamicAddress )
        {
            dynamicPool.push_back(*itPool);
            continue;
        }
        cout << "\tpool : " << (*itPool)._name << endl;
        cout << "\tvariable size : " << (*itPool)._data.size() << endl;
        if ( !(*itPool)._addressSize )
        {//Dynamic addressSize
            (*itPool)._addressSize = (*itPool)._data.size();
        }
        cout << "\taddress required : " << (*itPool)._addressMin << " to " << (*itPool)._addressMin+(*itPool)._addressSize << endl;
        unsigned short addressCount = (*itPool)._addressMin;
        for ( std::list<codeg::Variable>::iterator itVar=(*itPool)._data.begin(); itVar!=(*itPool)._data.end(); ++itVar )
        {
            cout << "\tvariable <"<<(*itVar)._name<<"> at " << addressCount << " with " << (*itVar)._link.size() << " links" << endl;
            for (unsigned int iLink=0; iLink<(*itVar)._link.size(); ++iLink)
            {
                (*itVar)._link[iLink][1] = addressCount&0x00FF;
                (*itVar)._link[iLink][3] = addressCount>>8;
            }
            ++addressCount;
        }
    }
    cout << "second pass, dynamic address" << endl;
    cout << "\tsize : " << dynamicPool.size() << endl;
    for ( std::list<codeg::Pool>::iterator itPool=dynamicPool.begin(); itPool!=dynamicPool.end(); ++itPool )
    {
        cout << "\tpool : " << (*itPool)._name << endl;
        cout << "\tvariable size : " << (*itPool)._data.size() << endl;
        (*itPool)._addressMin = 0;
        (*itPool)._addressSize = (*itPool)._data.size();
        redoCheck:
        //Finding a free address space
        for (std::list<codeg::Pool>::iterator it=codePoolList._data.begin(); it!=codePoolList._data.end(); ++it)
        {//Check if address space is not reserved
            if ( (!(*it)._dynamicAddress) )
            {
                if ( ((*itPool)._addressMin>=(*it)._addressMin) && ((*itPool)._addressMin<=((*it)._addressMin+(*it)._addressSize)) )
                {
                    ++(*itPool)._addressMin;
                    goto redoCheck;
                }
                if ( ((*itPool)._addressMin+(*itPool)._addressSize>=(*it)._addressMin) && (((*itPool)._addressMin+(*itPool)._addressSize)<=((*it)._addressMin+(*it)._addressSize)) )
                {
                    ++(*itPool)._addressMin;
                    goto redoCheck;
                }
                if ( ((*itPool)._addressMin<=(*it)._addressMin) && (((*itPool)._addressMin+(*itPool)._addressSize)>=(*it)._addressMin) )
                {
                    ++(*itPool)._addressMin;
                    goto redoCheck;
                }
                break;
            }
        }

        cout << "\taddress required : " << (*itPool)._addressMin << " to " << (*itPool)._addressMin+(*itPool)._addressSize << endl;
        unsigned short addressCount = (*itPool)._addressMin;
        for ( std::list<codeg::Variable>::iterator itVar=(*itPool)._data.begin(); itVar!=(*itPool)._data.end(); ++itVar )
        {
            cout << "\tvariable <"<<(*itVar)._name<<"> at " << addressCount << " with " << (*itVar)._link.size() << " links" << endl;
            for (unsigned int iLink=0; iLink<(*itVar)._link.size(); ++iLink)
            {
                (*itVar)._link[iLink][1] = addressCount&0x00FF;
                (*itVar)._link[iLink][3] = addressCount>>8;
            }
            ++addressCount;
        }
    }
    cout << "Resolving address pool completed !" << endl << endl;
**/








#if 0
PoolList::PoolList()
{
    this->_totalUsedSize=0;
}
PoolList::~PoolList()
{
}

bool PoolList::addPool(codeg::Pool& newPool)
{
    for (std::list<codeg::Pool>::iterator it=this->_data.begin(); it!=this->_data.end(); ++it)
    {
        if ( (*it)._name == newPool._name )
        {
            return false;
        }
    }

    newPool._data.clear();

    if ( newPool._dynamicAddress )
    {//Dynamic address space
        if ( !newPool._addressSize )
        {//Dynamic size
            this->_data.push_back(newPool);
            return true;
        }
        else
        {//Fixed size
            this->_data.push_back(newPool);
            return true;
        }
    }
    else
    {//Fixed address space
        if ( !newPool._addressSize )
        {//Dynamic size
            return false;
        }
        else
        {//Fixed size
            for (std::list<codeg::Pool>::iterator it=this->_data.begin(); it!=this->_data.end(); ++it)
            {//Check if address space is not reserved
                if ( (!(*it)._dynamicAddress) )
                {
                    if ( (newPool._addressMin>=(*it)._addressMin) && (newPool._addressMin<=((*it)._addressMin+(*it)._addressSize)) )
                    {
                        return false;
                    }
                    if ( (newPool._addressMin+newPool._addressSize>=(*it)._addressMin) && ((newPool._addressMin+newPool._addressSize)<=((*it)._addressMin+(*it)._addressSize)) )
                    {
                        return false;
                    }
                    if ( (newPool._addressMin<=(*it)._addressMin) && ((newPool._addressMin+newPool._addressSize)>=(*it)._addressMin) )
                    {
                        return false;
                    }
                }
            }

            this->_data.push_back(newPool);
            return true;
        }
    }
}
codeg::Pool* PoolList::getPool(const std::string& poolName)
{
    for (std::list<codeg::Pool>::iterator it=this->_data.begin(); it!=this->_data.end(); ++it)
    {
        if ( (*it)._name == poolName )
        {
            return &(*it);
        }
    }
    return nullptr;
}

bool PoolList::addVar(const std::string& poolName, codeg::Variable& newVar)
{
    for (std::list<codeg::Pool>::iterator it=this->_data.begin(); it!=this->_data.end(); ++it)
    {
        if ( (*it)._name == poolName )
        {
            for (std::list<codeg::Variable>::iterator varit=(*it)._data.begin(); varit!=(*it)._data.end(); ++varit)
            {
                if ( (*varit)._name == newVar._name )
                {
                    this->_lastError = "\""+newVar._name+"\" already exist !";
                    return false;
                }
            }

            newVar._link.clear();

            if ( (*it)._dynamicAddress )
            {
                if ( (*it)._addressSize )
                {
                    if ( ((*it)._data.size()+1) > (*it)._addressSize )
                    {
                        this->_lastError = "pool <"+(*it)._name+"> is overloaded, max size is : "+std::to_string((*it)._addressSize)+" !";
                        return false;
                    }
                }
            }
            else
            {
                if ( ((*it)._data.size()+1) > (*it)._addressSize )
                {
                    this->_lastError = "pool <"+(*it)._name+"> is overloaded, max size is : "+std::to_string((*it)._addressSize)+" !";
                    return false;
                }
            }

            (*it)._data.push_back(newVar);
            return true;
        }
    }

    this->_lastError = "pool <"+poolName+"> is not declared !";
    return false;
}
codeg::Variable* PoolList::getVar(const std::string& poolName, const std::string& varName)
{
    for (std::list<codeg::Pool>::iterator it=this->_data.begin(); it!=this->_data.end(); ++it)
    {
        if ( (*it)._name == poolName )
        {
            for (std::list<codeg::Variable>::iterator varit=(*it)._data.begin(); varit!=(*it)._data.end(); ++varit)
            {
                if ( (*varit)._name == varName )
                {
                    return &(*varit);
                }
            }
        }
    }
    return nullptr;
}

bool PoolList::addLink(const std::string& poolName, const std::string& varName, char* varLink)
{
    for (std::list<codeg::Pool>::iterator it=this->_data.begin(); it!=this->_data.end(); ++it)
    {
        if ( (*it)._name == poolName )
        {
            for (std::list<codeg::Variable>::iterator varit=(*it)._data.begin(); varit!=(*it)._data.end(); ++varit)
            {
                if ( (*varit)._name == varName )
                {
                    (*varit)._link.push_back(varLink);
                    return true;
                }
            }
        }
    }
    return false;
}

bool PoolList::isVariable(const std::string& str)
{
    if (str.size()>1)
    {
        return str[0] == '$';
    }
    return false;
}
#endif

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

bool ReadOnRam(const std::string& poolName, const std::string& varName, codeg::PoolList& poolList, std::vector<char>& codeData, unsigned int& cursor)
{
    #if 0
    codeg::Variable* buffVar = poolList.getVar(poolName, varName);
    if (!buffVar)
    {
        return false;
    }
    buffVar->_link.push_back( &codeData[cursor] );
    codeData[cursor++] = OPC_BRAMADD1_CLK|RV_SRCVALUE;
    codeData[cursor++] = 0x00;
    codeData[cursor++] = OPC_BRAMADD2_CLK|RV_SRCVALUE;
    codeData[cursor++] = 0x00;
    #endif
    return true;
}
void ReadOnRam(const unsigned short address, std::vector<char>& codeData, unsigned int& cursor)
{
    #if 0
    codeData[cursor++] = OPC_BRAMADD1_CLK|RV_SRCVALUE;
    codeData[cursor++] = address&0x00FF;
    codeData[cursor++] = OPC_BRAMADD2_CLK|RV_SRCVALUE;
    codeData[cursor++] = (address&0xFF00)>>8;
    #endif // 0
}

}//end codeg
