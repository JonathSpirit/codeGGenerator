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

#ifndef C_ERROR_H_INCLUDED
#define C_ERROR_H_INCLUDED

#include <exception>

namespace codeg
{

class CompileError : public std::exception
{
public:
    CompileError(const std::string& err)
    {
        this->g_str = err;
    }

    virtual const char* what() const noexcept
    {
        return this->g_str.c_str();
    }

private:
    std::string g_str;
};

class FatalError : public std::exception
{
public:
    FatalError(const std::string& err)
    {
        this->g_str = err;
    }

    virtual const char* what() const noexcept
    {
        return this->g_str.c_str();
    }

private:
    std::string g_str;
};

class SyntaxError : public std::exception
{
public:
    SyntaxError(const std::string& err)
    {
        this->g_str = err;
    }

    virtual const char* what() const noexcept
    {
        return this->g_str.c_str();
    }

private:
    std::string g_str;
};

}//end codeg

#endif // C_ERROR_H_INCLUDED
