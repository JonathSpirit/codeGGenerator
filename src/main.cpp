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

#include "main.hpp"

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <limits>

#include "C_target.hpp"
#include "C_variable.hpp"
#include "C_compilerData.hpp"
#include "C_console.hpp"
#include "C_error.hpp"

#include "CMakeConfig.hpp"

void printHelp()
{
    std::cout << "codeGGenerator usage :" << std::endl << std::endl;

    std::cout << "Set the input file to be compiled" << std::endl;
    std::cout << "\tcodeGGenerator --in=<path>" << std::endl << std::endl;

    std::cout << "Set the output file (default is the input path+.cg)" << std::endl;
    std::cout << "\tcodeGGenerator --out=<path>" << std::endl << std::endl;

    std::cout << "Set the output log file (default is the input path+.log)" << std::endl;
    std::cout << "\tcodeGGenerator --outLog=<path>" << std::endl << std::endl;

    std::cout << "Don't write a log file (default a log file is written)" << std::endl;
    std::cout << "\tcodeGGenerator --noLog" << std::endl << std::endl;

    std::cout << "Write dummy arguments/values (useful for old compatibility), default no" << std::endl;
    std::cout << "\tcodeGGenerator --writeDummy" << std::endl << std::endl;

    std::cout << "Print the version (and do nothing else)" << std::endl;
    std::cout << "\tcodeGGenerator --version" << std::endl << std::endl;

    std::cout << "Print the help page (and do nothing else)" << std::endl;
    std::cout << "\tcodeGGenerator --help" << std::endl << std::endl;

    std::cout << "Ask the user how he want to compile his file (interactive compiling)" << std::endl;
    std::cout << "\tcodeGGenerator --ask" << std::endl << std::endl;
}
void printVersion()
{
    std::cout << "codeGGenerator created by Guillaume Guillet, version " << CGG_VERSION_MAJOR << "." << CGG_VERSION_MINOR << std::endl;
}

int main(int argc, char **argv)
{
    if ( int err = codeg::ConsoleInit() )
    {
        std::cout << "Warning, bad console init, the console can be ugly now ! (error: "<<err<<")" << std::endl;
    }

    std::filesystem::path fileInPath;
    std::filesystem::path fileOutPath;
    std::filesystem::path fileReadableOutPath;
    std::filesystem::path fileLogOutPath;

    std::vector<std::string> commands(argv, argv + argc);

    if (commands.size() <= 1)
    {
        printHelp();
        return -1;
    }

    bool writeDummy = false;
    bool writeLogFile = true;

    for (std::size_t i=1; i<commands.size(); ++i)
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
        if ( commands[i] == "--noLog")
        {
            writeLogFile = false;
            continue;
        }
        if ( commands[i] == "--writeDummy")
        {
            writeDummy = true;
            continue;
        }
        if ( commands[i] == "--ask")
        {
            std::cout << "Please insert the input path of the file"<< std::endl <<"> ";
            std::string path;
            std::getline(std::cin, path);
            fileInPath = path;
            continue;
        }

        //Commands with an argument
        std::vector<std::string> splitCommand;
        codeg::Split(commands[i], splitCommand, '=');

        if (splitCommand.size() == 2)
        {
            if (splitCommand[0] == "--in")
            {
                fileInPath = splitCommand[1];
                continue;
            }
            if (splitCommand[0] == "--out")
            {
                fileOutPath = splitCommand[1];
                continue;
            }
            if (splitCommand[0] == "--outLog")
            {
                fileLogOutPath = splitCommand[1];
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
        fileOutPath = fileInPath;
        fileOutPath += ".cg";
    }
    fileReadableOutPath = fileInPath;
    fileReadableOutPath += ".rcg";
    if ( fileLogOutPath.empty() && writeLogFile )
    {
        fileLogOutPath = fileInPath;
        fileLogOutPath += ".log";
    }

    ///Compiler data
    codeg::CompilerData data;
    data._code.setWriteDummy(writeDummy);

    ///Opening files
    if ( !data._reader.open( std::shared_ptr<codeg::ReaderData>(new codeg::ReaderData_file(fileInPath)) ) )
    {
        std::cout << "Can't read the file "<< fileInPath << std::endl;
        return -1;
    }
    data._relativePath = fileInPath.relative_path().parent_path();

    std::ofstream fileOutBinary( fileOutPath, std::ios::binary | std::ios::trunc );
    if ( !fileOutBinary )
    {
        std::cout << "Can't write the file "<< fileOutPath << std::endl;
        return -1;
    }
    std::ofstream fileOutReadable( fileReadableOutPath, std::ios::trunc );
    if ( !fileOutReadable )
    {
        std::cout << "Can't write the file "<< fileInPath << std::endl;
        return -1;
    }

    codeg::varConsole = new codeg::Console();
    if (writeLogFile)
    {
        if ( !codeg::varConsole->logOpen(fileLogOutPath) )
        {
            std::cout << "Can't write the file "<< fileLogOutPath << std::endl;
            return -1;
        }
    }

    std::cout << "Input file : "<< fileInPath << std::endl;
    std::cout << "Output file : "<< fileOutPath << std::endl;
    if (writeLogFile)
    {
        std::cout << "Output log file : "<< fileLogOutPath << std::endl;
    }
    std::cout << std::endl;

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
    data._reservedKeywords.push("select");
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
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_set()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_unset()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_var()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_label()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_jump()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_restart()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_affect()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_get()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_write()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_select()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_do()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_tick()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_brut()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_function()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_if()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_else()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_ifnot()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_end()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_call()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_clock()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_pool()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_import()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_definition()) );
    data._instructions.push( std::unique_ptr<codeg::Instruction>(new codeg::Instruction_enddef()) );

    ///Code
    data._code.resize(65536);

    std::string readLine;

    try
    {
        ///First step reading and compiling
        ConsoleInfo << "Step 1 : Reading and compiling ..." << std::endl;

        while( data._reader.getline(readLine) )
        {
            data._decomposer.decompose(readLine, data._decomposer._flags);

            if ( !data._decomposer._instruction.empty() )
            {
                codeg::Instruction* instruction = data._instructions.get( data._decomposer._instruction );

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
                    throw codeg::FatalError("unknown instruction \""+data._decomposer._instruction+"\"");
                }
            }
        }

        if ( !data._scopes.empty() )
        {//A scope is not terminated by 'end'
            throw codeg::CompileError("scope without an 'end' (maybe at line: "+std::to_string(data._scopes.top()._startLine)+" and file: "+data._scopes.top()._startFile+")");
        }

        ConsoleInfo << "Step 1 : OK !\n" << std::endl;
        ConsoleInfo << "Compiled size : " << data._code.getCursor() << " bytes\n" << std::endl;

        ///Second step resolving jumplist
        ConsoleInfo << "Step 2 : Resolving jumpList ..." << std::endl;

        data._jumps.resolve(data);

        ConsoleInfo << "Step 2 : OK !\n" << std::endl;

        ///Third step resolving pools
        ConsoleInfo << "Step 3 : Resolving pools ..." << std::endl;

        codeg::MemoryBigSize totalSize = data._pools.resolve(data);

        if (totalSize > std::numeric_limits<codeg::MemorySize>::max())
        {
            throw codeg::CompileError("data overflow, with "+std::to_string(totalSize)+" bytes");
        }
        ConsoleInfo << "Memory used size : " << totalSize << " bytes\n" << std::endl;

        ConsoleInfo << "Step 3 : OK !\n" << std::endl;

        ///Writing on the output file
        ConsoleInfo << "Writing codeG file (binary size : " << data._code.getCursor() << " bytes) ..." << std::endl;
        fileOutBinary.write(reinterpret_cast<char*>(data._code.getData()), data._code.getCursor());
        fileOutBinary.close();

        ConsoleInfo << "Writing readable codeG file ..." << std::endl;

        bool tmpFlagOpcode = true;
        uint8_t* dataPtr = data._code.getData();
        for (uint32_t i = 0; i<data._code.getCursor(); ++i)
        {
            fileOutReadable << "["<<codeg::ValueToHex(dataPtr[i], 2)<<"]";

            if ( tmpFlagOpcode )
            {
                if ( ((dataPtr[i]&CODEG_BINARYOPCODES_MASK) != codeg::OPCODE_JMPSRC_CLK) &&
                     ((dataPtr[i]&CODEG_READABLEBUSSES_MASK) == codeg::READABLE_SOURCE) )
                {//The jump instruction does not have an argument
                    tmpFlagOpcode = false;
                }
                fileOutReadable << " " << codeg::OpcodeToString(dataPtr[i]) << " <" << codeg::ReadableBusToString(dataPtr[i]) << ">";
            }
            else
            {
                tmpFlagOpcode = true;
            }

            fileOutReadable << std::endl;
        }

        fileOutReadable.close();
        ConsoleInfo << "OK !\n" << std::endl;
    }
    catch (const codeg::CompileError& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            ConsoleError << "at file " << data._reader.getPath() << std::endl;
        }
        if (tmpLine > 0)
        {
            ConsoleError << "at line " << data._reader.getlineCount() << " : " << e.what() << std::endl;
        }
        else
        {
            ConsoleError << e.what() << std::endl;
        }
        return -1;
    }
    catch (const codeg::FatalError& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            ConsoleFatal << "at file " << data._reader.getPath() << std::endl;
        }
        if (tmpLine > 0)
        {
            ConsoleFatal << "at line " << data._reader.getlineCount() << " : " << e.what() << std::endl;
        }
        else
        {
            ConsoleFatal << e.what() << std::endl;
        }
        return -1;
    }
    catch (const codeg::SyntaxError& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            ConsoleSyntax << "at file " << data._reader.getPath() << std::endl;
        }
        if (tmpLine > 0)
        {
            ConsoleSyntax << "at line " << data._reader.getlineCount() << " : " << e.what() << std::endl;
        }
        else
        {
            ConsoleSyntax << e.what() << std::endl;
        }
        return -1;
    }
    catch (const std::exception& e)
    {
        std::string tmpPath = data._reader.getPath();
        unsigned int tmpLine = data._reader.getlineCount();
        if (!tmpPath.empty())
        {
            ConsoleFatal << "at file " << data._reader.getPath() << std::endl;
        }
        if (tmpLine > 0)
        {
            ConsoleFatal << "at line " << data._reader.getlineCount() << " : unknown exception : " << e.what() << std::endl;
        }
        else
        {
            ConsoleFatal << "unknown exception : " << e.what() << std::endl;
        }
        return -1;
    }

    codeg::varConsole->logClose();
    delete codeg::varConsole;

    return 0;
}
