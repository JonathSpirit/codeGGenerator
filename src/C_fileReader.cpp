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

#include "C_fileReader.hpp"

namespace codeg
{

FileReader::FileReader()
{
}
FileReader::~FileReader()
{
}

void FileReader::closeAll()
{
    unsigned int stackSize = this->g_files.size();

    for (unsigned int i=0; i<stackSize; ++i)
    {
        this->g_files.top()._file.close();
        this->g_files.pop();
    }
}

bool FileReader::open(const std::string& path)
{
    std::ifstream file( path );
    if ( !file )
    {
        return false;
    }
    this->g_files.push( {std::move(file), 0} );
    return true;
}
bool FileReader::getline(std::string& buffLine)
{
    if (!this->g_files.size())
    {
        return false;
    }

    if ( std::getline(this->g_files.top()._file, buffLine) )
    {
        ++this->g_files.top()._lineCount;
        return true;
    }
    this->g_files.top()._file.close();
    this->g_files.pop();

    if ( this->g_files.size() > 0 )
    {
        buffLine.clear();
        return true;
    }
    return false;
}
unsigned int FileReader::getlineCount() const
{
    if ( this->g_files.size() )
    {
        return this->g_files.top()._lineCount;
    }
    return 0;
}
unsigned int FileReader::getSize() const
{
    return this->g_files.size();
}

}//end codeg
