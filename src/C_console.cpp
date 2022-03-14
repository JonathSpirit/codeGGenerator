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

#include "C_console.hpp"
#include <iostream>
#include <fstream>

#ifdef _WIN32
    #include <windows.h>

    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#endif

namespace codeg
{

bool Console::logOpen(const std::filesystem::path& path)
{
    if ( this->g_log.is_open() )
    {
        return false;
    }
    this->g_log.rdbuf()->pubsetbuf(nullptr,0);
    this->g_log.open(path, std::ofstream::ate);

    if (this->g_log)
    {
        return true;
    }
    this->g_log.close();
    return false;
}
void Console::logClose()
{
    this->g_log.close();
}

codeg::Console* varConsole{nullptr};

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

}//end codeg
