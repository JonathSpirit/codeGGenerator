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

#include "C_console.h"
#include <iostream>
#include <ctime>
#include <iomanip>

#include <wchar.h>

#ifdef _WIN32
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace codeg
{

int ConsoleInit()
{
    #ifdef _WIN32
    ///WINDOWS

    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return GetLastError();
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return GetLastError();
    }
    #endif

    return 0;
}

void ConsoleWrite(const std::string& str)
{
    std::cout << str << std::endl;
}

void ConsoleFatalWrite(const std::string& str)
{
    std::time_t t = std::time(nullptr);
    std::cout << "\x1b[31m";
    std::cout << "[fatal](" << std::put_time(std::localtime(&t), "%d.%m.%Y - %H:%M:%S") << ") " << str << std::endl;
    std::cout << "\x1b[0m";
}

void ConsoleErrorWrite(const std::string& str)
{
    std::time_t t = std::time(nullptr);
    std::cout << "\x1b[31m";
    std::cout << "[error](" << std::put_time(std::localtime(&t), "%d.%m.%Y - %H:%M:%S") << ") " << str << std::endl;
    std::cout << "\x1b[0m";
}

void ConsoleWarningWrite(const std::string& str)
{
    std::time_t t = std::time(nullptr);
    std::cout << "\x1b[34m";
    std::cout << "[warning](" << std::put_time(std::localtime(&t), "%d.%m.%Y - %H:%M:%S") << ") " << str << std::endl;
    std::cout << "\x1b[0m";
}

void ConsoleInfoWrite(const std::string& str)
{
    std::time_t t = std::time(nullptr);
    std::cout << "[info](" << std::put_time(std::localtime(&t), "%d.%m.%Y - %H:%M:%S") << ") " << str << std::endl;
}

}//end codeg
