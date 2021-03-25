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

#include "main.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <string>

#include "C_fileReader.hpp"
#include "C_target.hpp"
#include "C_value.hpp"
#include "C_variable.hpp"
#include "C_keyword.hpp"
#include "C_compilerData.hpp"
#include "C_console.hpp"

#include "CMakeConfig.hpp"

using namespace std;

void printHelp()
{
    std::cout << "codeGGcompiler usage :" << std::endl << std::endl;

    std::cout << "Set the input file to be compiled" << std::endl;
    std::cout << "\tcodeGGcompiler --in=<path>" << std::endl << std::endl;

    std::cout << "Set the output file (default is the input path+.cg)" << std::endl;
    std::cout << "\tcodeGGcompiler --out=<path>" << std::endl << std::endl;

    std::cout << "Print the version (and do nothing else)" << std::endl;
    std::cout << "\tcodeGGcompiler --version" << std::endl << std::endl;

    std::cout << "Print the help page (and do nothing else)" << std::endl;
    std::cout << "\tcodeGGcompiler --help" << std::endl << std::endl;

    std::cout << "Ask the user how he want to compile his file (interactive compiling)" << std::endl;
    std::cout << "\tcodeGGcompiler --ask" << std::endl << std::endl;
}
void printVersion()
{
    std::cout << "codeGGcompiler created by Guillaume Guillet, version " << CGG_VERSION_MAJOR << "." << CGG_VERSION_MINOR << std::endl;
}

int main(int argc, char **argv)
{
    if ( int err = codeg::ConsoleInit() )
    {
        std::cout << "Warning, bad console init, the console can be ugly now ! (error: "<<err<<")" << std::endl;
    }

    std::string fileInPath;
    std::string fileOutPath;

    std::vector<std::string> commands(argv, argv + argc);

    if (commands.size() <= 1)
    {
        printHelp();
        return -1;
    }

    for (unsigned int i=1; i<commands.size(); ++i)
    {
        //Commands
        if ( commands[i] == "--help")
        {
            printHelp();
            return 0;
        }
        if ( commands[i] == "--version")
        {
            printVersion();
            return 0;
        }
        if ( commands[i] == "--ask")
        {
            std::cout << "Please insert the input path of the file"<< std::endl <<"> ";
            std::getline(std::cin, fileInPath);
            continue;
        }

        //Commands with an argument
        std::vector<std::string> splitedCommand;
        codeg::Split(commands[i], splitedCommand, '=');

        if (splitedCommand.size() == 2)
        {
            if ( splitedCommand[0] == "--in")
            {
                fileInPath = splitedCommand[1];
                continue;
            }
            if ( splitedCommand[0] == "--out")
            {
                fileOutPath = splitedCommand[1];
                continue;
            }
        }

        //Unknown command
        std::cout << "Unknown command : \""<< commands[i] <<"\" !" << std::endl;
        return -1;
    }

    if ( fileInPath.empty() )
    {
        std::cout << "No input file !" << std::endl;
        return -1;
    }
    if ( fileOutPath.empty() )
    {
        fileOutPath = fileInPath+".cg";
    }

    ///Opening files
    codeg::FileReader reader;

    if ( !reader.open(fileInPath) )
    {
        std::cout << "Can't read the file \""<< fileInPath <<"\"" << std::endl;
        return -1;
    }

    std::ofstream fileOutBinary( fileOutPath, std::ios::binary | std::ios::trunc );
    if ( !fileOutBinary )
    {
        std::cout << "Can't write the file \""<< fileOutPath <<".cg\"" << std::endl;
        return -1;
    }
    std::ofstream fileOutReadable( fileInPath+".rcg", std::ios::trunc );
    if ( !fileOutReadable )
    {
        std::cout << "Can't write the file \""<< fileInPath <<".rcg\"" << std::endl;
        return -1;
    }

    std::cout << "Input file : \""<< fileInPath <<"\"" << std::endl;
    std::cout << "Output file : \""<< fileOutPath <<"\"" << std::endl;

    ///Creating default pool
    codeg::Pool defaultPool("global");
    defaultPool.setStartAddressType(codeg::Pool::StartAddressTypes::START_ADDRESS_DYNAMIC);
    defaultPool.setAddress(0x00, 0x0000);

    ///Compiler data
    codeg::CompilerData data;

    ///Set default pool
    data._defaultPool = "global";
    data._pools.addPool(defaultPool);

    ///Reserved keywords
    data._reservedKeywords.push_back("set");
    data._reservedKeywords.push_back("var");
    data._reservedKeywords.push_back("label");
    data._reservedKeywords.push_back("affect");
    data._reservedKeywords.push_back("function");
    data._reservedKeywords.push_back("do");
    data._reservedKeywords.push_back("if_not");
    data._reservedKeywords.push_back("else");
    data._reservedKeywords.push_back("end");
    data._reservedKeywords.push_back("choose");
    data._reservedKeywords.push_back("OP");
    data._reservedKeywords.push_back("P");
    data._reservedKeywords.push_back("write");
    data._reservedKeywords.push_back("if");
    data._reservedKeywords.push_back("brut");
    data._reservedKeywords.push_back("jump");
    data._reservedKeywords.push_back("call");
    data._reservedKeywords.push_back("restart");
    data._reservedKeywords.push_back("PERIPHERAL");
    data._reservedKeywords.push_back("OPERATION");
    data._reservedKeywords.push_back("tick");
    data._reservedKeywords.push_back("simple");
    data._reservedKeywords.push_back("long");
    data._reservedKeywords.push_back("repeat");
    data._reservedKeywords.push_back("_src");
    data._reservedKeywords.push_back("_bread1");
    data._reservedKeywords.push_back("_bread2");
    data._reservedKeywords.push_back("_result");
    data._reservedKeywords.push_back("_ram");
    data._reservedKeywords.push_back("_spi");
    data._reservedKeywords.push_back("_ext1");
    data._reservedKeywords.push_back("_ext2");
    data._reservedKeywords.push_back("pool");
    data._reservedKeywords.push_back("#");
    data._reservedKeywords.push_back("#[");
    data._reservedKeywords.push_back("]#");
    data._reservedKeywords.push_back("SPI");

    ///Instructions
    data._instructions.push_back(new codeg::Instruction_set());
    data._instructions.push_back(new codeg::Instruction_var());
    data._instructions.push_back(new codeg::Instruction_label());
    data._instructions.push_back(new codeg::Instruction_jump());
    data._instructions.push_back(new codeg::Instruction_restart());
    data._instructions.push_back(new codeg::Instruction_affect());
    data._instructions.push_back(new codeg::Instruction_write());
    data._instructions.push_back(new codeg::Instruction_choose());
    data._instructions.push_back(new codeg::Instruction_do());
    data._instructions.push_back(new codeg::Instruction_tick());
    data._instructions.push_back(new codeg::Instruction_brut());
    data._instructions.push_back(new codeg::Instruction_function());
    data._instructions.push_back(new codeg::Instruction_if());
    data._instructions.push_back(new codeg::Instruction_else());
    data._instructions.push_back(new codeg::Instruction_ifnot());
    data._instructions.push_back(new codeg::Instruction_end());
    data._instructions.push_back(new codeg::Instruction_call());
    data._instructions.push_back(new codeg::Instruction_clock());
    data._instructions.push_back(new codeg::Instruction_pool());

    ///Code
    data._code.resize(65536);

    std::string readedLine;
    bool validInstruction = false;

    try
    {
        ///First step reading and compiling
        codeg::ConsoleInfoWrite("Step 1 : Reading and compiling ...");

        while( reader.getline(readedLine) )
        {
            data._decomposer.decompose(readedLine, data._decomposer._flags);

            if (data._decomposer._keywords.size() > 0)
            {
                validInstruction = false;

                for (auto&& instruction : data._instructions)
                {
                    if (instruction->getName() == data._decomposer._keywords[0])
                    {
                        validInstruction = true;
                        instruction->compile(data._decomposer, data);
                        break;
                    }
                }

                if (!validInstruction)
                {
                    throw codeg::FatalError("unknown instruction \""+data._decomposer._keywords[0]+"\"");
                }
            }
        }

        codeg::ConsoleInfoWrite("Step 1 : OK !\n");
        codeg::ConsoleInfoWrite("Compiled size : "+std::to_string(data._code._cursor)+" bytes\n");

        ///Second step resolving jumplist
        codeg::ConsoleInfoWrite("Step 2 : Resolving jumpList ...");

        data._jumps.resolve(data);

        codeg::ConsoleInfoWrite("Step 2 : OK !\n");

        ///Third step resolving pools
        codeg::ConsoleInfoWrite("Step 3 : Resolving pools ...");

        data._pools.resolve(data);

        codeg::ConsoleInfoWrite("Step 3 : OK !\n");

        ///Writing on the output file
        codeg::ConsoleInfoWrite("Writing codeG file (binary size : "+std::to_string(data._code._cursor)+" bytes) ...");
        fileOutBinary.write(reinterpret_cast<char*>(data._code._data.get()), data._code._cursor);
        fileOutBinary.close();

        codeg::ConsoleInfoWrite("Writing readable codeG file ...");

        bool tmpFlagOpcode = true;
        for (uint32_t i = 0; i<data._code._cursor; ++i)
        {
            fileOutReadable << "["<<codeg::Uint8ToHex(data._code._data[i])<<"]";

            if ( tmpFlagOpcode )
            {
                if ((data._code._data[i]&0x1F) != codeg::OPCODE_JMPSRC_CLK)
                {//The jump instruction does not have an argument
                    tmpFlagOpcode = false;
                }
                fileOutReadable << " " << ToReadableOpcode(data._code._data[i]) << " <" << ToReadableBus(data._code._data[i]) << ">";
            }
            else
            {
                tmpFlagOpcode = true;
            }

            fileOutReadable << std::endl;
        }

        fileOutReadable.close();
        codeg::ConsoleInfoWrite("OK !\n");
    }
    catch (const codeg::CompileError& e)
    {
        codeg::ConsoleErrorWrite("at line "+std::to_string(reader.getlineCount())+" : "+e.what());
        return -1;
    }
    catch (const codeg::FatalError& e)
    {
        codeg::ConsoleFatalWrite("at line "+std::to_string(reader.getlineCount())+" : "+e.what());
        return -1;
    }
    catch (const std::exception& e)
    {
        codeg::ConsoleFatalWrite("at line "+std::to_string(reader.getlineCount())+" : unknown exception : "+std::string(e.what()) );
        return -1;
    }

    return 0;
}
