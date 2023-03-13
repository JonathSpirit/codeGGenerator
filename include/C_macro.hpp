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

#ifndef C_MACRO_HPP_INCLUDED
#define C_MACRO_HPP_INCLUDED

#include <string>
#include <unordered_map>
#include <functional>
#include <optional>

#define CODEG_MAX_REPLACEMENT_COUNT 100

namespace codeg
{

class MacroList
{
public:
    using ListType = std::unordered_map<std::string, std::string>;

    MacroList() = default;
    ~MacroList() = default;

    void clear();

    bool replace(std::string& str) const;

    void set(const std::string& key, const std::string& str);

    bool remove(const std::string& key);
    bool check(const std::string& key) const;

private:
    codeg::MacroList::ListType g_data;
};

class InlinedStaticMacroList
{
public:
    using ListType = std::unordered_map<std::string, std::function<std::optional<std::string>(const std::string&)> >;

    InlinedStaticMacroList() = default;
    ~InlinedStaticMacroList() = default;

    void clear();

    std::optional<std::string> getReplacement(const std::string& str, const std::string& arg) const;

    void set(const std::string& key, std::function<std::optional<std::string>(const std::string&)> func);

    bool remove(const std::string& key);
    bool check(const std::string& key) const;

private:
    codeg::InlinedStaticMacroList::ListType g_data;
};

}//end codeg

#endif // C_MACRO_HPP_INCLUDED
