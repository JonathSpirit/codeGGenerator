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

#include "C_fileReader.hpp"

namespace codeg
{

///ReaderData
ReaderData::ReaderData()
{
}
ReaderData::~ReaderData()
{

}

void ReaderData::setlineCount(unsigned int n)
{
    this->_g_lineCount = n;
}
void ReaderData::addlineCount(unsigned int n)
{
    this->_g_lineCount += n;
}

unsigned int ReaderData::getlineCount() const
{
    return this->_g_lineCount;
}
const std::string& ReaderData::getPath() const
{
    return this->_g_path;
}

///ReaderData_file
ReaderData_file::ReaderData_file() : ReaderData()
{

}
ReaderData_file::ReaderData_file(const std::filesystem::path& filePath)
{
    this->g_file.open(filePath);
    this->_g_lineCount = 0;
    this->_g_path = filePath.string();
}
ReaderData_file::~ReaderData_file()
{

}

bool ReaderData_file::getline(std::string& buffLine)
{
    return static_cast<bool>(std::getline(this->g_file, buffLine));
}
bool ReaderData_file::isValid() const
{
    return this->g_file.good();
}
void ReaderData_file::close()
{
    this->g_file.close();
}

std::ifstream& ReaderData_file::getstream()
{
    return this->g_file;
}

///ReaderData_definition
ReaderData_definition::ReaderData_definition()
{

}
ReaderData_definition::ReaderData_definition(const codeg::Function* func)
{
    this->g_func = func;
    this->g_it = this->g_func->getIteratorBegin();

    this->_g_lineCount = 0;
    this->_g_path = "\"definition call: "+func->getName()+"\"";
}
ReaderData_definition::~ReaderData_definition()
{

}

bool ReaderData_definition::getline(std::string& buffLine)
{
    if (this->g_it != this->g_func->getIteratorEnd())
    {
        buffLine = *this->g_it;
        ++this->g_it;
        return true;
    }
    return false;
}
bool ReaderData_definition::isValid() const
{
    return this->g_func->isDefinition();
}
void ReaderData_definition::close()
{

}

const codeg::Function* ReaderData_definition::getfunction()
{
    return this->g_func;
}

///FileReader
FileReader::FileReader()
{
}
FileReader::~FileReader()
{
    this->closeAll();
}

void FileReader::closeAll()
{
    unsigned int stackSize = this->g_data.size();

    for (unsigned int i=0; i<stackSize; ++i)
    {
        this->g_data.top()->close();
        this->g_data.pop();
    }
}

bool FileReader::open(std::shared_ptr<codeg::ReaderData> newData)
{
    if ( newData->isValid() )
    {
        this->g_data.push(newData);
        return true;
    }
    return false;
}
bool FileReader::getline(std::string& buffLine)
{
    if (!this->g_data.size())
    {
        return false;
    }

    if ( this->g_data.top()->getline(buffLine) )
    {
        this->g_data.top()->addlineCount();
        return true;
    }
    this->g_data.top()->close();
    this->g_data.pop();

    if ( this->g_data.size() > 0 )
    {
        buffLine.clear();
        return true;
    }
    return false;
}
unsigned int FileReader::getlineCount() const
{
    if ( this->g_data.size() )
    {
        return this->g_data.top()->getlineCount();
    }
    return 0;
}
unsigned int FileReader::getSize() const
{
    return this->g_data.size();
}
std::string FileReader::getPath() const
{
    if ( this->g_data.size() )
    {
        return this->g_data.top()->getPath();
    }
    return "";
}

}//end codeg
