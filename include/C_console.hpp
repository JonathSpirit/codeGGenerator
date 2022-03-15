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

#ifndef C_CONSOLE_HPP_INCLUDED
#define C_CONSOLE_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iomanip>

#define ConsoleNone *codeg::varConsole

#define ConsoleFatal *codeg::varConsole << codeg::ConsoleOutputType::OUTPUT_FATAL
#define ConsoleError *codeg::varConsole << codeg::ConsoleOutputType::OUTPUT_ERROR
#define ConsoleWarning *codeg::varConsole << codeg::ConsoleOutputType::OUTPUT_WARNING
#define ConsoleSyntax *codeg::varConsole << codeg::ConsoleOutputType::OUTPUT_SYNTAX
#define ConsoleInfo *codeg::varConsole << codeg::ConsoleOutputType::OUTPUT_INFO

namespace codeg
{

enum ConsoleOutputType
{
    OUTPUT_NONE,

    OUTPUT_FATAL,
    OUTPUT_ERROR,
    OUTPUT_WARNING,
    OUTPUT_SYNTAX,
    OUTPUT_INFO
};

class Console
{
public:
    using CharT = std::ostream::char_type;
    using Traits = std::char_traits<CharT>;

    Console() = default;
    ~Console() = default;

    bool logOpen(const std::filesystem::path& path);
    void logClose();

    codeg::Console& operator <<( std::basic_ostream<CharT,Traits>& (*func)(std::basic_ostream<CharT,Traits>&) )
    {
        if (func == &std::endl<CharT,Traits> )
        {
            std::cout << "\x1b[0m\n";
            if (this->g_log)
            {
                this->g_log << '\n';
            }
        }
        return *this;
    }

    template<class T>
    codeg::Console& operator <<(const T& val)
    {
        if constexpr ( std::is_same<T, codeg::ConsoleOutputType>::value )
        {
            std::time_t t = std::time(nullptr);
            auto localTime = std::put_time(std::localtime(&t), "%d.%m.%Y - %H:%M:%S");

            switch ( static_cast<codeg::ConsoleOutputType>(val) )
            {
            case OUTPUT_FATAL:
                std::cout << "\x1b[31m";
                std::cout << "[fatal](" << localTime << ") ";
                if (this->g_log)
                {
                    this->g_log << "[fatal](" << localTime << ") ";
                }
                break;
            case OUTPUT_ERROR:
                std::cout << "\x1b[31m";
                std::cout << "[error](" << localTime << ") ";
                if (this->g_log)
                {
                    this->g_log << "[error](" << localTime << ") ";
                }
                break;
            case OUTPUT_WARNING:
                std::cout << "\x1b[36m";
                std::cout << "[warning](" << localTime << ") ";
                if (this->g_log)
                {
                    this->g_log << "[warning](" << localTime << ") ";
                }
                break;
            case OUTPUT_SYNTAX:
                std::cout << "\x1b[33m";
                std::cout << "[syntax error](" << localTime << ") ";
                if (this->g_log)
                {
                    this->g_log << "[syntax error](" << localTime<< ") ";
                }
                break;
            case OUTPUT_INFO:
                std::cout << "[info](" << localTime << ") ";
                if (this->g_log)
                {
                    this->g_log << "[info](" << localTime << ") ";
                }
                break;
            default:
                break;
            }
        }
        else
        {
            std::cout << val;
            if (this->g_log)
            {
                this->g_log << val;
            }
        }

        return *this;
    }

private:
    std::ofstream g_log;
};

extern codeg::Console* varConsole;

int ConsoleInit();

}//end codeg

#endif // C_CONSOLE_HPP_INCLUDED
