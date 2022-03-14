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
ReaderData::ReaderData(std::string path) :
        _g_path(std::move(path))
{
}

void ReaderData::setLineCount(std::size_t n)
{
    this->_g_lineCount = n;
}
void ReaderData::addLineCount(std::size_t n)
{
    this->_g_lineCount += n;
}

std::size_t ReaderData::getLineCount() const
{
    return this->_g_lineCount;
}
const std::string& ReaderData::getPath() const
{
    return this->_g_path;
}

///ReaderData_file
ReaderData_file::ReaderData_file(const std::filesystem::path& filePath) :
        codeg::ReaderData(filePath.string()),
        g_file(filePath.string())
{
}

bool ReaderData_file::getLine(std::string& buffLine)
{
    return static_cast<bool>(std::getline(this->g_file, buffLine));
}
bool ReaderData_file::isValid() const
{
    return static_cast<bool>(this->g_file);
}
void ReaderData_file::close()
{
    this->g_file.close();
}

std::ifstream& ReaderData_file::getStream()
{
    return this->g_file;
}

///ReaderData_definition
ReaderData_definition::ReaderData_definition(const codeg::Function* func) :
        codeg::ReaderData("\"definition call: "+func->getName()+"\""),
        g_func(func),
        g_it(func->getIteratorBegin())
{
}

bool ReaderData_definition::getLine(std::string& buffLine)
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

const codeg::Function* ReaderData_definition::getFunction()
{
    return this->g_func;
}

///FileReader
FileReader::~FileReader()
{
    this->closeAll();
}

void FileReader::closeAll()
{
    std::size_t stackSize = this->g_data.size();

    for (std::size_t i=0; i<stackSize; ++i)
    {
        this->g_data.top()->close();
        this->g_data.pop();
    }
}

bool FileReader::open(std::shared_ptr<codeg::ReaderData> newData)
{
    if ( newData && newData->isValid() )
    {
        this->g_data.push(std::move(newData) );
        return true;
    }
    return false;
}
bool FileReader::getLine(std::string& buffLine)
{
    if (this->g_data.empty())
    {
        return false;
    }

    if (this->g_data.top()->getLine(buffLine) )
    {
        this->g_data.top()->addLineCount();
        return true;
    }
    this->g_data.top()->close();
    this->g_data.pop();

    if ( !this->g_data.empty() )
    {
        buffLine.clear();
        return true;
    }
    return false;
}
std::size_t FileReader::getLineCount() const
{
    if (!this->g_data.empty())
    {
        return this->g_data.top()->getLineCount();
    }
    return 0;
}
std::size_t FileReader::getSize() const
{
    return this->g_data.size();
}
std::string FileReader::getPath() const
{
    if ( !this->g_data.empty() )
    {
        return this->g_data.top()->getPath();
    }
    return {};
}

}//end codeg
