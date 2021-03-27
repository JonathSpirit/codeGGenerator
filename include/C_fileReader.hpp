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

#include "C_function.hpp"
#include <fstream>
#include <stack>
#include <string>
#include <memory>

namespace codeg
{

class ReaderData
{
public:
    ReaderData();
    virtual ~ReaderData() = 0;

    virtual bool getline(std::string& buffLine) = 0;
    virtual bool isValid() const = 0;
    virtual void close() = 0;

    void setlineCount(unsigned int n);
    void addlineCount(unsigned int n = 1);

    unsigned int getlineCount() const;
    const std::string& getPath() const;

protected:
    unsigned int _g_lineCount;
    std::string _g_path;
};

class ReaderData_file : public ReaderData
{
public:
    ReaderData_file();
    ReaderData_file(const std::string& filePath);
    ~ReaderData_file();

    bool getline(std::string& buffLine);
    bool isValid() const;
    void close();

    std::ifstream& getstream();

private:
    std::ifstream g_file;
};

class ReaderData_definition : public ReaderData
{
public:
    ReaderData_definition();
    ReaderData_definition(const codeg::Function* func);
    ~ReaderData_definition();

    bool getline(std::string& buffLine);
    bool isValid() const;
    void close();

    const codeg::Function* getfunction();

private:
    const codeg::Function* g_func;
    std::list<std::string>::const_iterator g_it;
};


class FileReader
{
public:
    FileReader();
    ~FileReader();

    void closeAll();

    bool open(std::shared_ptr<codeg::ReaderData> newData);

    bool getline(std::string& buffLine);
    unsigned int getlineCount() const;
    std::string getPath() const;

    unsigned int getSize() const;

private:
    std::stack<std::shared_ptr<codeg::ReaderData> > g_data;
};

}//end codeg

#endif // C_FILEREADER_H_INCLUDED
