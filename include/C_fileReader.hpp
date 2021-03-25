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

#ifndef C_FILEREADER_H_INCLUDED
#define C_FILEREADER_H_INCLUDED

#include <fstream>
#include <stack>
#include <string>

namespace codeg
{

class FileReader
{
public:
    struct Data
    {
        std::ifstream _file;
        unsigned int _lineCount;
        std::string _path;
    };

    FileReader();
    ~FileReader();

    void closeAll();

    bool open(const std::string& path);

    bool getline(std::string& buffLine);
    unsigned int getlineCount() const;
    unsigned int getSize() const;
    std::string getPath() const;

private:
    std::stack<codeg::FileReader::Data> g_files;
};

}//end codeg

#endif // C_FILEREADER_H_INCLUDED
