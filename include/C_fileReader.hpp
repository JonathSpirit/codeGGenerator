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

#ifndef C_FILEREADER_HPP_INCLUDED
#define C_FILEREADER_HPP_INCLUDED

#include "C_function.hpp"
#include <fstream>
#include <stack>
#include <string>
#include <memory>
#include <filesystem>

namespace codeg
{

class ReaderData
{
public:
    ReaderData() = default;
    explicit ReaderData(std::string path);
    virtual ~ReaderData() = default;

    virtual bool getLine(std::string& buffLine) = 0;
    [[nodiscard]] virtual bool isValid() const = 0;
    virtual void close() = 0;

    void setLineCount(std::size_t n);
    void addLineCount(std::size_t n = 1);

    [[nodiscard]] std::size_t getLineCount() const;
    [[nodiscard]] const std::string& getPath() const;

protected:
    std::size_t _g_lineCount{0};
    std::string _g_path;
};

class ReaderData_file : public ReaderData
{
public:
    ReaderData_file() = default;
    explicit ReaderData_file(const std::filesystem::path& filePath);
    ~ReaderData_file() override = default;

    bool getLine(std::string& buffLine) override;
    [[nodiscard]] bool isValid() const override;
    void close() override;

    std::ifstream& getStream();

private:
    std::ifstream g_file;
};

class ReaderData_definition : public ReaderData
{
public:
    ReaderData_definition() = default;
    explicit ReaderData_definition(const codeg::Function* func);
    ~ReaderData_definition() override = default;

    bool getLine(std::string& buffLine) override;
    [[nodiscard]] bool isValid() const override;
    void close() override;

    const codeg::Function* getFunction();

private:
    const codeg::Function* g_func{nullptr};
    std::list<std::string>::const_iterator g_it;
};

class FileReader
{
public:
    FileReader() = default;
    ~FileReader();

    void closeAll();

    bool open(std::shared_ptr<codeg::ReaderData> newData);

    bool getLine(std::string& buffLine);
    [[nodiscard]] std::size_t getLineCount() const;
    [[nodiscard]] std::string getPath() const;

    [[nodiscard]] std::size_t getSize() const;

private:
    std::stack<std::shared_ptr<codeg::ReaderData> > g_data;
};

}//end codeg

#endif // C_FILEREADER_HPP_INCLUDED
