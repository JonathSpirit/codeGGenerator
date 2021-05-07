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

#include "C_target.hpp"
#include "C_value.hpp"
#include "C_variable.hpp"
#include "C_keyword.hpp"
#include "C_compilerData.hpp"
#include "C_console.hpp"
#include "C_error.hpp"

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

    ///Compiler data
    codeg::CompilerData data;

    ///Opening files
    if ( !data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_file(fileInPath)) ) )
    {
        std::cout << "Can't read the file \""<< fileInPath <<"\"" << std::endl;
        return -1;
    }
    data._relativePath = codeg::GetRelativePath(fileInPath);

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

    ///Set default pool
    data._defaultPool = "global";
    data._pools.addPool(defaultPool);

    ///Reserved keywords
    data._reservedKeywords.push("set");
    data._reservedKeywords.push("unset");
    data._reservedKeywords.push("var");
    data._reservedKeywords.push("label");
    data._reservedKeywords.push("affect");
    data._reservedKeywords.push("get");
    data._reservedKeywords.push("function");
    data._reservedKeywords.push("do");
    data._reservedKeywords.push("if_not");
    data._reservedKeywords.push("else");
    data._reservedKeywords.push("end");
    data._reservedKeywords.push("choose");
    data._reservedKeywords.push("OP");
    data._reservedKeywords.push("P");
    data._reservedKeywords.push("write");
    data._reservedKeywords.push("if");
    data._reservedKeywords.push("brut");
    data._reservedKeywords.push("jump");
    data._reservedKeywords.push("call");
    data._reservedKeywords.push("restart");
    data._reservedKeywords.push("PERIPHERAL");
    data._reservedKeywords.push("OPERATION");
    data._reservedKeywords.push("tick");
    data._reservedKeywords.push("simple");
    data._reservedKeywords.push("long");
    data._reservedKeywords.push("repeat");
    data._reservedKeywords.push("_src");
    data._reservedKeywords.push("_bread1");
    data._reservedKeywords.push("_bread2");
    data._reservedKeywords.push("_result");
    data._reservedKeywords.push("_ram");
    data._reservedKeywords.push("_spi");
    data._reservedKeywords.push("_ext1");
    data._reservedKeywords.push("_ext2");
    data._reservedKeywords.push("pool");
    data._reservedKeywords.push("#");
    data._reservedKeywords.push("#[");
    data._reservedKeywords.push("]#");
    data._reservedKeywords.push("SPI");
    data._reservedKeywords.push("import");
    data._reservedKeywords.push("definition");
    data._reservedKeywords.push("end_def");

    ///Instructions
    data._instructions.push(new codeg::Instruction_set());
    data._instructions.push(new codeg::Instruction_unset());
    data._instructions.push(new codeg::Instruction_var());
    data._instructions.push(new codeg::Instruction_label());
    data._instructions.push(new codeg::Instruction_jump());
    data._instructions.push(new codeg::Instruction_restart());
    data._instructions.push(new codeg::Instruction_affect());
    data._instructions.push(new codeg::Instruction_get());
    data._instructions.push(new codeg::Instruction_write());
    data._instructions.push(new codeg::Instruction_choose());
    data._instructions.push(new codeg::Instruction_do());
    data._instructions.push(new codeg::Instruction_tick());
    data._instructions.push(new codeg::Instruction_brut());
    data._instructions.push(new codeg::Instruction_function());
    data._instructions.push(new codeg::Instruction_if());
    data._instructions.push(new codeg::Instruction_else());
    data._instructions.push(new codeg::Instruction_ifnot());
    data._instructions.push(new codeg::Instruction_end());
    data._instructions.push(new codeg::Instruction_call());
    data._instructions.push(new codeg::Instruction_clock());
    data._instructions.push(new codeg::Instruction_pool());
    data._instructions.push(new codeg::Instruction_import());
    data._instructions.push(new codeg::Instruction_definition());
    data._instructions.push(new codeg::Instruction_enddef());

    ///Code
    data._code.resize(65536);

    std::string readedLine;

    try
    {
        ///First step reading and compiling
        codeg::ConsoleInfoWrite("Step 1 : Reading and compiling ...");

        while( data._reader.getline(readedLine) )
        {
            data._decomposer.decompose(readedLine, data._decomposer._flags);

            if (data._decomposer._keywords.size() > 0)
            {
                codeg::Instruction* instruction = data._instructions.get( data._decomposer._keywords[0] );

                if (instruction != nullptr)
                {//Instruction founded
                    if ( data._writeLinesIntoDefinition )
                    {//Compile in a definition (detect the end_def keyword)
                        instruction->compileDefinition(data._decomposer, data);
                    }
                    else
                    {//Compile
                        instruction->compile(data._decomposer, data);
                    }
                }
                else
                {//Bad instruction
                    throw codeg::FatalError("unknown instruction \""+data._decomposer._keywords[0]+"\"");
                }
            }
        }

        if ( data._scope.size() > 0 )
        {//A scope is not terminated by 'end'
            throw codeg::CompileError("scope without an 'end' (maybe at line: "+std::to_string(data._scope.top()._startLine)+" and file: "+data._scope.top()._startFile+")");
        }

        codeg::ConsoleInfoWrite("Step 1 : OK !\n");
        codeg::ConsoleInfoWrite("Compiled size : "+std::to_string(data._code.getCursor())+" bytes\n");

        ///Second step resolving jumplist
        codeg::ConsoleInfoWrite("Step 2 : Resolving jumpList ...");

        data._jumps.resolve(data);

        codeg::ConsoleInfoWrite("Step 2 : OK !\n");

        ///Third step resolving pools
        codeg::ConsoleInfoWrite("Step 3 : Resolving pools ...");

        data._pools.resolve(data);

        codeg::ConsoleInfoWrite("Step 3 : OK !\n");

        ///Writing on the output file
        codeg::ConsoleInfoWrite("Writing codeG file (binary size : "+std::to_string(data._code.getCursor())+" bytes) ...");
        fileOutBinary.write(reinterpret_cast<char*>(data._code.getData()), data._code.getCursor());
        fileOutBinary.close();

        codeg::ConsoleInfoWrite("Writing readable codeG file ...");

        bool tmpFlagOpcode = true;
        uint8_t* dataPtr = data._code.getData();
        for (uint32_t i = 0; i<data._code.getCursor(); ++i)
        {
            fileOutReadable << "["<<codeg::ValueToHex(dataPtr[i], 2)<<"]";

            if ( tmpFlagOpcode )
            {
                if ((dataPtr[i]&0x1F) != codeg::OPCODE_JMPSRC_CLK)
                {//The jump instruction does not have an argument
                    tmpFlagOpcode = false;
                }
                fileOutReadable << " " << ToReadableOpcode(dataPtr[i]) << " <" << ToReadableBus(dataPtr[i]) << ">";
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
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            codeg::ConsoleErrorWrite("at file "+data._reader.getPath());
        }
        if (tmpLine > 0)
        {
            codeg::ConsoleErrorWrite("at line "+std::to_string(data._reader.getlineCount())+" : "+e.what());
        }
        else
        {
            codeg::ConsoleErrorWrite(e.what());
        }
        return -1;
    }
    catch (const codeg::FatalError& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            codeg::ConsoleFatalWrite("at file "+data._reader.getPath());
        }
        if (tmpLine > 0)
        {
            codeg::ConsoleFatalWrite("at line "+std::to_string(data._reader.getlineCount())+" : "+e.what());
        }
        else
        {
            codeg::ConsoleFatalWrite(e.what());
        }
        return -1;
    }
    catch (const codeg::SyntaxError& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            codeg::ConsoleSyntaxWrite("at file "+data._reader.getPath());
        }
        if (tmpLine > 0)
        {
            codeg::ConsoleSyntaxWrite("at line "+std::to_string(data._reader.getlineCount())+" : "+e.what());
        }
        else
        {
            codeg::ConsoleSyntaxWrite(e.what());
        }
        return -1;
    }
    catch (const std::exception& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            codeg::ConsoleFatalWrite("at file "+data._reader.getPath());
        }
        if (tmpLine > 0)
        {
            codeg::ConsoleFatalWrite("at line "+std::to_string(data._reader.getlineCount())+" : unknown exception : "+std::string(e.what()) );
        }
        else
        {
            codeg::ConsoleFatalWrite("unknown exception : "+std::string(e.what()));
        }
        return -1;
    }

    return 0;
}
