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
#include <sstream>
#include <stack>
#include <map>
#include <vector>
#include <string>
#include <thread>

#include "C_target.hpp"
#include "C_value.hpp"
#include "C_variable.hpp"
#include "C_keyword.hpp"
#include "C_compilerData.hpp"
#include "C_console.hpp"

using namespace std;

std::vector<std::string> _global_bannedKeywords;

std::string ToStrHexa(unsigned int val)
{
    char buff[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    std::string out;
    out = "x";
    unsigned int mask = 0xF0000000;
    for (unsigned int i=0; i<8; ++i)
    {
        out += buff[(val&mask) >> 4*(7-i)];
        mask >>= 4;
    }

    return out;
}

LineError::LineError(const std::string& err)
{
    this->g_errstr = err;
}

const char* LineError::what() const noexcept
{
    return this->g_errstr.c_str();
}

int main()
{
    if ( codeg::ConsoleInit() )
    {
        std::cout << "Warning, bad console init, the console can be ugly now !" << std::endl;
    }

    std::string fileInPath;
    std::cout << "Please insert the input path of the file"<< std::endl <<"> ";
    std::getline(std::cin, fileInPath);

    std::ifstream fileIn( fileInPath );
    if ( !fileIn )
    {
        std::cout << "Can't read the file \""<< fileInPath <<"\"" << std::endl;
        return -1;
    }

    std::ofstream fileOutBinary( fileInPath+".cg", std::ios::binary | std::ios::trunc );
    if ( !fileOutBinary )
    {
        std::cout << "Can't write the file \""<< fileInPath <<".cg\"" << std::endl;
        return -1;
    }
    std::ofstream fileOutReadable( fileInPath+".rcg", std::ios::trunc );
    if ( !fileOutReadable )
    {
        std::cout << "Can't write the file \""<< fileInPath <<".rcg\"" << std::endl;
        return -1;
    }

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
    uint32_t linePosition = 0;
    bool validInstruction = false;

    try
    {
        ///First step reading and compiling
        codeg::ConsoleInfoWrite("Step 1 : Reading and compiling ...");

        while( std::getline(fileIn, readedLine) )
        {
            ++linePosition;

            data._decomposer.decompose(readedLine, data._decomposer._flags);

            //std::cout << "line["<<linePosition<<"] " << data._decomposer._cleaned << std::endl;

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
        codeg::ConsoleInfoWrite("Step 4 : Writing the files ...");
        codeg::ConsoleInfoWrite("Binary size : "+std::to_string(data._code._cursor)+" Bytes");

        fileOutBinary.write(reinterpret_cast<char*>(data._code._data.get()), data._code._cursor);
        fileOutBinary.close();

        fileOutReadable.close();
        codeg::ConsoleInfoWrite("Step 4 : OK !\n");
    }
    catch (const codeg::CompileError& e)
    {
        codeg::ConsoleErrorWrite("at line "+std::to_string(linePosition)+" : "+e.what());
    }
    catch (const codeg::FatalError& e)
    {
        codeg::ConsoleFatalWrite(e.what());
    }
    catch (const std::exception& e)
    {
        codeg::ConsoleFatalWrite( "unknown exception : "+std::string(e.what()) );
    }

    #if 0
    _global_bannedKeywords.reserve(21);
    _global_bannedKeywords.push_back("set");
    _global_bannedKeywords.push_back("var");
    _global_bannedKeywords.push_back("label");
    _global_bannedKeywords.push_back("affect");
    _global_bannedKeywords.push_back("function");
    _global_bannedKeywords.push_back("do");
    _global_bannedKeywords.push_back("write");
    _global_bannedKeywords.push_back("if");
    _global_bannedKeywords.push_back("if_not");
    _global_bannedKeywords.push_back("else_end");
    _global_bannedKeywords.push_back("end");
    _global_bannedKeywords.push_back("else");
    _global_bannedKeywords.push_back("choose");
    _global_bannedKeywords.push_back("OP");
    _global_bannedKeywords.push_back("P");
    _global_bannedKeywords.push_back("PERIPHERAL");
    _global_bannedKeywords.push_back("OPERATION");
    _global_bannedKeywords.push_back("brut");
    _global_bannedKeywords.push_back("jump");
    _global_bannedKeywords.push_back("call");

    std::string buffLine;
    unsigned int fileLine = 0;

    codeg::KeywordsList buffKeywords;

    codeg::Value buffValue;
    unsigned int buffBus;

    unsigned int codeCursor = 0;
    unsigned int codeAllocSize = 65'536;
    std::vector<char> codeResult;
    codeResult.resize(codeAllocSize);

    codeg::PoolList codePoolList;
    codeg::Pool buffPool;
    codeg::Variable buffVariable;
    codeg::CustomKeywordsList codeCustomKeyword;
    std::string codeActualPool = "global";
    std::map<std::string, std::vector<char*>> codeJumpPoints;
    std::map<std::string, unsigned int> codeLabels;
    std::vector<std::string> codeFunctions;
    std::string codeActualFunction;
    unsigned int codeConditionCount=0;
    std::stack<unsigned int> codeConditionRest;
///Scope, liste des pools, default pool,
///jumpPoints, labels, functions, instruction
    bool flagCodeInFunction=false;

    buffPool = {"global", true, 0, 0};
    codePoolList.addPool(buffPool);

    try
    {
    while( std::getline(file, buffLine) )
    {
        ++fileLine;

        if ( codeg::GetKeywordsFromString(buffLine, buffKeywords) )
        {
            codeg::ReplaceWithCustomKeywords(buffKeywords, codeCustomKeyword);

            if ( buffKeywords[0] == "write" )
            {///write [bus] [value]
                if ( buffKeywords.size() == 3 )
                {
                    if ( !codeg::GetIntegerFromString(buffKeywords[1], buffBus) )
                    {
                        throw LineError("invalid [value] for argument 1 [bus] ("+buffKeywords[1]+")");
                    }
                    buffValue = codeg::ProcessValue(buffKeywords[2], codeActualPool, codePoolList);

                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        break;
                    default: throw LineError("invalid value type !");
                    }

                    switch (buffBus)
                    {
                    case 1:
                        codeResult[codeCursor++] = OPC_BWRITE1_CLK|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    case 2:
                        codeResult[codeCursor++] = OPC_BWRITE2_CLK|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid argument 1 [bus], must be 1 or 2");
                    }
                    continue;
                }
                else
                {
                    throw LineError("number of argument invalid ! Correct form is \"write [bus] [value]\"");
                }
            }
            if ( buffKeywords[0] == "choose" )
            {///choose [target] [value]
                if ( buffKeywords.size() == 3 )
                {
                    buffValue = codeg::ProcessValue(buffKeywords[2], "global", codePoolList);

                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        if ( codeg::ProcessTarget(buffKeywords[1]) == codeg::TargetType::TARGET_PERIPHERAL )
                        {//codeg::TargetType::TARGETTYPE_PERIPHERAL
                            codeResult[codeCursor++] = OPC_BPCS_CLK|buffValue._readValue;
                        }
                        else
                        {//codeg::TargetType::TARGETTYPE_OPERATION
                            codeResult[codeCursor++] = OPC_OPC_CLK|buffValue._readValue;
                        }
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }
                    continue;
                }
                else
                {
                    throw LineError("number of argument invalid ! Correct form is \"choose [target] [value]\"");
                }
            }
            if ( buffKeywords[0] == "set" )
            {///set [keyword] [string]
                if ( buffKeywords.size() == 3 )
                {
                    if ( codeCustomKeyword.find(buffKeywords[1]) != codeCustomKeyword.end() )
                    {//Already exist
                        cout << "Warning at line " << fileLine << " : keyword \""<< buffKeywords[1]
                             <<"\" already set with value ("<< codeCustomKeyword[buffKeywords[1]] <<") ! (replaced)" << endl;
                    }
                    codeCustomKeyword[buffKeywords[1]] = buffKeywords[2];
                    continue;
                }
                else
                {
                    throw LineError("number of argument invalid ! Correct form is \"set [keyword] [value]\"");
                }
            }
            if ( buffKeywords[0] == "var" )
            {///var [keyword] ([keyword])
                if ( buffKeywords.size() == 3 )
                {//Custom pool
                    buffVariable = {buffKeywords[1]};
                    if ( !codePoolList.addVar(buffKeywords[2], buffVariable) )
                    {
                        throw LineError(codePoolList._lastError);
                    }
                    continue;
                }
                if ( buffKeywords.size() == 2 )
                {//Default pool
                    buffVariable = {buffKeywords[1]};
                    if ( !codePoolList.addVar(codeActualPool, buffVariable) )
                    {
                        throw LineError(codePoolList._lastError);
                    }
                    continue;
                }
                throw LineError("number of argument invalid ! Correct form is \"var [keyword] ([keyword])\"");
            }
            if ( buffKeywords[0] == "label" )
            {///label [keyword]
                if ( buffKeywords.size() == 2 )
                {
                    if ( codeLabels.find(buffKeywords[1]) != codeLabels.end() )
                    {
                        throw LineError("label \""+buffKeywords[1]+"\" already declared !");
                    }
                    codeLabels[buffKeywords[1]] = codeCursor;
                    continue;
                }
                throw LineError("number of argument invalid ! Correct form is \"label [keyword]\"");
            }
            if ( buffKeywords[0] == "affect" )
            {///affect [variable] [value]
                if ( buffKeywords.size() == 3 )
                {
                    buffValue = codeg::ProcessValue(buffKeywords[1], codeActualPool, codePoolList);
                    if ( buffValue._type != codeg::ValueType::VALUETYPE_VARIABLE )
                    {
                        throw LineError("\""+buffKeywords[1]+"\" is not a variable !");
                    }
                    buffValue._variable->_link.push_back( &codeResult[codeCursor] );
                    codeResult[codeCursor++] = OPC_BRAMADD1_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BRAMADD2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;

                    buffValue = codeg::ProcessValue(buffKeywords[2], codeActualPool, codePoolList);

                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        throw LineError("\"affect\" doesn't support, for now, copy between variable !");
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        codeResult[codeCursor++] = OPC_RAMW|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }
                    continue;
                }
                else
                {
                    throw LineError("number of argument invalid ! Correct form is \"affect [variable] [value]\"");
                }
            }
            if ( buffKeywords[0] == "function" )
            {///function [keyword]
                if ( buffKeywords.size() == 2 )
                {
                    if ( flagCodeInFunction )
                    {
                        throw LineError("Function in a function !");
                    }
                    if ( std::find(codeFunctions.begin(), codeFunctions.end(), buffKeywords[1]) != codeFunctions.end() )
                    {
                        throw LineError("Function \""+buffKeywords[1]+"\" already declared !");
                    }
                    if ( codeLabels.find(buffKeywords[1]) != codeLabels.end() )
                    {
                        throw LineError("Label \""+buffKeywords[1]+"\" is already declared ! (a function always go with a label with the same name)");
                    }

                    flagCodeInFunction = true;
                    codeActualFunction = buffKeywords[1];
                    codeFunctions.push_back(buffKeywords[1]);

                    codeJumpPoints[" %"+buffKeywords[1]].push_back(&codeResult[codeCursor]);
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_JMPSRC_CLK;

                    codeLabels[buffKeywords[1]] = codeCursor;
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"function [keyword]\"");
            }
            if ( buffKeywords[0] == "do" )
            {///do [value] [value] [value]
                if ( buffKeywords.size() == 4 )
                {
                    //Left
                    buffValue = codeg::ProcessValue(buffKeywords[1], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        codeResult[codeCursor++] = OPC_OPLEFT_CLK|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }

                    //Choose
                    buffValue = codeg::ProcessValue(buffKeywords[2], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        codeResult[codeCursor++] = OPC_OPC_CLK|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }

                    //Right
                    buffValue = codeg::ProcessValue(buffKeywords[3], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        codeResult[codeCursor++] = OPC_OPRIGHT_CLK|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"do [value] [value] [value]\"");
            }
            if ( buffKeywords[0] == "if" )
            {///if [value]
                if ( buffKeywords.size() == 2 )
                {
                    codeConditionRest.push(++codeConditionCount);

                    codeJumpPoints[" c"+std::to_string(codeConditionRest.top())+"|false"].push_back(&codeResult[codeCursor]);
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE; //Jump to false
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;

                    buffValue = codeg::ProcessValue(buffKeywords[1], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        codeResult[codeCursor++] = OPC_IF|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }

                    codeResult[codeCursor++] = OPC_JMPSRC_CLK; //Jump to false
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"if [value]\"");
            }
            if ( buffKeywords[0] == "else" )
            {///else
                if ( buffKeywords.size() == 1 )
                {
                    if ( codeConditionRest.empty() )
                    {
                        throw LineError("\"else\" cannot be used in this context !");
                    }

                    codeJumpPoints[" c"+std::to_string(codeConditionRest.top())+"|end"].push_back(&codeResult[codeCursor]);
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE; //Jump to end
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_JMPSRC_CLK;

                    codeLabels[" c"+std::to_string(codeConditionRest.top())+"|false"] = codeCursor; //Label false
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"else\"");
            }
            if ( buffKeywords[0] == "end" )
            {///end
                if ( buffKeywords.size() == 1 )
                {
                    //Condition
                    if ( codeConditionRest.empty() )
                    {
                        //End of function
                        if ( flagCodeInFunction )
                        {
                            flagCodeInFunction = false;
                            codeLabels[" %"+codeActualFunction] = codeCursor;
                            continue;
                        }

                        throw LineError("\"end\" cannot be used in this context !");
                    }

                    if ( codeLabels.find(" c"+std::to_string(codeConditionRest.top())+"|false") != codeLabels.end() )
                    {//Label false already exist, we are in a else
                        codeLabels[" c"+std::to_string(codeConditionRest.top())+"|end"] = codeCursor; //Label end
                    }
                    else
                    {//Label false don't exist, we are not in a else
                        codeLabels[" c"+std::to_string(codeConditionRest.top())+"|false"] = codeCursor; //Label false
                    }
                    codeConditionRest.pop();
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"end\"");
            }
            if ( buffKeywords[0] == "if_not" )
            {///if_not [value]
                if ( buffKeywords.size() == 2 )
                {
                    codeConditionRest.push(++codeConditionCount);

                    codeJumpPoints[" c"+std::to_string(codeConditionRest.top())+"|false"].push_back(&codeResult[codeCursor]);
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE; //Jump to false
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;

                    buffValue = codeg::ProcessValue(buffKeywords[1], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        codeg::ReadOnRam(codeActualPool, buffValue._variable->_name, codePoolList, codeResult, codeCursor);
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        codeResult[codeCursor++] = OPC_IFNOT|buffValue._readValue;
                        codeResult[codeCursor++] = buffValue._value;
                        break;
                    default: throw LineError("invalid value type !");
                    }

                    codeResult[codeCursor++] = OPC_JMPSRC_CLK; //Jump to false
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"if_not [value]\"");
            }
            if ( buffKeywords[0] == "clock" )
            {///clock [value]
                if ( buffKeywords.size() == 2 )
                {
                    buffValue = codeg::ProcessValue(buffKeywords[1], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        throw LineError("\"clock\" doesn't support, for now, a variable !");
                        break;
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                        for (unsigned int i=0; i<buffValue._value; ++i)
                        {
                            codeResult[codeCursor++] = OPC_PERIPHERAL_CLK;
                            codeResult[codeCursor++] = buffValue._value;
                        }
                        break;
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        throw LineError("\"clock\" doesn't support, for now, a read value !");
                        break;
                    default: throw LineError("invalid value type !");
                    }
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"clock [value]\"");
            }
            if ( buffKeywords[0] == "tick" )
            {///tick ["simple"/"long"] ([value])
                if ( buffKeywords.size() == 2 )
                {
                    if (buffKeywords[1] == "simple")
                    {
                        codeResult[codeCursor++] = OPC_STICK;
                        codeResult[codeCursor++] = 0x00;
                    }
                    else if (buffKeywords[1] == "long")
                    {
                        codeResult[codeCursor++] = OPC_LTICK;
                        codeResult[codeCursor++] = 0x00;
                    }
                    else
                    {
                        throw LineError("invalid keyword <"+buffKeywords[2]+"> ! Keyword must be \"simple\" or \"long\"");
                    }
                    continue;
                }
                if ( buffKeywords.size() == 3 )
                {
                    buffValue = codeg::ProcessValue(buffKeywords[2], codeActualPool, codePoolList);
                    switch (buffValue._type)
                    {
                    case codeg::ValueType::VALUETYPE_VARIABLE:
                        throw LineError("\"tick\" doesn't support, for now, a variable !");
                        break;
                    case codeg::ValueType::VALUETYPE_CONSTANT:
                        if (buffKeywords[1] == "simple")
                        {
                            for (unsigned int i=0; i<buffValue._value; ++i)
                            {
                                codeResult[codeCursor++] = OPC_STICK;
                                codeResult[codeCursor++] = 0x00;
                            }
                        }
                        else if (buffKeywords[1] == "long")
                        {
                            for (unsigned int i=0; i<buffValue._value; ++i)
                            {
                                codeResult[codeCursor++] = OPC_LTICK;
                                codeResult[codeCursor++] = 0x00;
                            }
                        }
                        else
                        {
                            throw LineError("invalid keyword <"+buffKeywords[1]+"> ! Keyword must be \"simple\" or \"long\"");
                        }
                        break;
                    case codeg::ValueType::VALUETYPE_READ_VALUE:
                        throw LineError("\"tick\" doesn't support, for now, a read value !");
                        break;
                    default: throw LineError("invalid value type !");
                    }

                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"tick [\"simple\"/\"long\"] ([value])\"");
            }
            if ( buffKeywords[0] == "brut" )
            {///brut [value], ...
                if ( buffKeywords.size() >= 2 )
                {
                    for (unsigned int i=1; i<buffKeywords.size(); ++i)
                    {
                        buffValue = codeg::ProcessValue(buffKeywords[i], codeActualPool, codePoolList);
                        if (buffValue._type != codeg::ValueType::VALUETYPE_CONSTANT)
                        {
                            throw LineError("\"brut\" argument "+std::to_string(i)+" must be a 8bits constant !");
                        }
                        codeResult[codeCursor++] = buffValue._value;
                    }
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"brut [value], ...\"");
            }
            if ( buffKeywords[0] == "jump" )
            {///jump [keyword]
                if ( buffKeywords.size() == 2 )
                {
                    codeJumpPoints[buffKeywords[1]].push_back(&codeResult[codeCursor]);
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_JMPSRC_CLK;
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"jump [keyword]\"");
            }
            if ( buffKeywords[0] == "restart" )
            {///restart
                if ( buffKeywords.size() == 1 )
                {
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_JMPSRC_CLK;
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"restart\"");
            }
            if ( buffKeywords[0] == "call" )
            {///call [keyword]
                if ( buffKeywords.size() == 2 )
                {
                    if ( std::find(codeFunctions.begin(), codeFunctions.end(), buffKeywords[1]) == codeFunctions.end() )
                    {
                        throw LineError("Function \""+buffKeywords[1]+"\" not declared !");
                    }

                    codeJumpPoints[buffKeywords[1]].push_back(&codeResult[codeCursor]);
                    codeResult[codeCursor++] = OPC_BJMPSRC1_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC2_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_BJMPSRC3_CLK|RV_SRCVALUE;
                    codeResult[codeCursor++] = 0x00;
                    codeResult[codeCursor++] = OPC_JMPSRC_CLK;
                    continue;
                }

                throw LineError("number of argument invalid ! Correct form is \"jump [keyword]\"");
            }
            if ( buffKeywords[0] == "repeat" )
            {///repeat [value] ([variable])
                if ( buffKeywords.size() == 2 )
                {
                    throw LineError("Functionality not implemented yet !");
                }
                if ( buffKeywords.size() == 1 )
                {
                    throw LineError("Functionality not implemented yet !");
                }

                throw LineError("number of argument invalid ! Correct form is \"repeat [value] ([variable])\"");
            }

            throw LineError("unknown keyword ("+buffKeywords[0]+") !");
        }
    }
    }
    catch (std::exception& e)
    {
        file.close();
        file_compiled.close();
        cout << "Error at line " << fileLine << " : " << e.what() << endl;
        return -1;
    }

    cout << "\nCompilation completed !" << endl << endl;
    cout << "Resolving address pool ..." << endl;
    cout << "first pass, fixed address" << endl;
    cout << "\tsize : " << codePoolList._data.size() << endl;
    std::list<codeg::Pool> dynamicPool;
    for ( std::list<codeg::Pool>::iterator itPool=codePoolList._data.begin(); itPool!=codePoolList._data.end(); ++itPool )
    {
        if ( (*itPool)._dynamicAddress )
        {
            dynamicPool.push_back(*itPool);
            continue;
        }
        cout << "\tpool : " << (*itPool)._name << endl;
        cout << "\tvariable size : " << (*itPool)._data.size() << endl;
        if ( !(*itPool)._addressSize )
        {//Dynamic addressSize
            (*itPool)._addressSize = (*itPool)._data.size();
        }
        cout << "\taddress required : " << (*itPool)._addressMin << " to " << (*itPool)._addressMin+(*itPool)._addressSize << endl;
        unsigned short addressCount = (*itPool)._addressMin;
        for ( std::list<codeg::Variable>::iterator itVar=(*itPool)._data.begin(); itVar!=(*itPool)._data.end(); ++itVar )
        {
            cout << "\tvariable <"<<(*itVar)._name<<"> at " << addressCount << " with " << (*itVar)._link.size() << " links" << endl;
            for (unsigned int iLink=0; iLink<(*itVar)._link.size(); ++iLink)
            {
                (*itVar)._link[iLink][1] = addressCount&0x00FF;
                (*itVar)._link[iLink][3] = addressCount>>8;
            }
            ++addressCount;
        }
    }
    cout << "second pass, dynamic address" << endl;
    cout << "\tsize : " << dynamicPool.size() << endl;
    for ( std::list<codeg::Pool>::iterator itPool=dynamicPool.begin(); itPool!=dynamicPool.end(); ++itPool )
    {
        cout << "\tpool : " << (*itPool)._name << endl;
        cout << "\tvariable size : " << (*itPool)._data.size() << endl;
        (*itPool)._addressMin = 0;
        (*itPool)._addressSize = (*itPool)._data.size();
        redoCheck:
        //Finding a free address space
        for (std::list<codeg::Pool>::iterator it=codePoolList._data.begin(); it!=codePoolList._data.end(); ++it)
        {//Check if address space is not reserved
            if ( (!(*it)._dynamicAddress) )
            {
                if ( ((*itPool)._addressMin>=(*it)._addressMin) && ((*itPool)._addressMin<=((*it)._addressMin+(*it)._addressSize)) )
                {
                    ++(*itPool)._addressMin;
                    goto redoCheck;
                }
                if ( ((*itPool)._addressMin+(*itPool)._addressSize>=(*it)._addressMin) && (((*itPool)._addressMin+(*itPool)._addressSize)<=((*it)._addressMin+(*it)._addressSize)) )
                {
                    ++(*itPool)._addressMin;
                    goto redoCheck;
                }
                if ( ((*itPool)._addressMin<=(*it)._addressMin) && (((*itPool)._addressMin+(*itPool)._addressSize)>=(*it)._addressMin) )
                {
                    ++(*itPool)._addressMin;
                    goto redoCheck;
                }
                break;
            }
        }

        cout << "\taddress required : " << (*itPool)._addressMin << " to " << (*itPool)._addressMin+(*itPool)._addressSize << endl;
        unsigned short addressCount = (*itPool)._addressMin;
        for ( std::list<codeg::Variable>::iterator itVar=(*itPool)._data.begin(); itVar!=(*itPool)._data.end(); ++itVar )
        {
            cout << "\tvariable <"<<(*itVar)._name<<"> at " << addressCount << " with " << (*itVar)._link.size() << " links" << endl;
            for (unsigned int iLink=0; iLink<(*itVar)._link.size(); ++iLink)
            {
                (*itVar)._link[iLink][1] = addressCount&0x00FF;
                (*itVar)._link[iLink][3] = addressCount>>8;
            }
            ++addressCount;
        }
    }
    cout << "Resolving address pool completed !" << endl << endl;
    cout << "Resolving jump points ..." << endl;
    for ( std::map<std::string, std::vector<char*>>::iterator it=codeJumpPoints.begin(); it!=codeJumpPoints.end(); ++it )
    {
        unsigned int address = codeLabels[it->first];
        cout << "\t" << it->second.size() << " jump points with the label : " << it->first << endl;
        for (unsigned int i=0; i<it->second.size(); ++i)
        {
            it->second[i][1] = address&0x000000FF;
            it->second[i][3] = (address&0x0000FF00)>>8;
            it->second[i][5] = (address&0x00FF0000)>>16;
        }
    }
    cout << "Resolving jump points completed !" << endl << endl;

    cout << "Total code size : " << codeCursor << endl;
    cout << "Saving to " << file_path+".cg ..." << endl;

    if ( !file_compiled.write(codeResult.data(), codeCursor) )
    {
        cout << "Error while saving to " << file_path+".cg !" << endl;
        return -1;
    }
    std::vector<std::string> nameKeyword;
    nameKeyword.reserve(25);
    nameKeyword.push_back("OPC_BWRITE1_CLK");
    nameKeyword.push_back("OPC_BWRITE2_CLK");
    nameKeyword.push_back("OPC_BPCS_CLK");
    nameKeyword.push_back("OPC_OPLEFT_CLK");
    nameKeyword.push_back("OPC_OPRIGHT_CLK");
    nameKeyword.push_back("OPC_OPC_CLK");
    nameKeyword.push_back("OPC_PERIPHERAL_CLK");
    nameKeyword.push_back("OPC_BJMPSRC1_CLK");
    nameKeyword.push_back("OPC_BJMPSRC2_CLK");
    nameKeyword.push_back("OPC_BJMPSRC3_CLK");
    nameKeyword.push_back("OPC_JMPSRC_CLK");
    nameKeyword.push_back("OPC_BRAMADD1_CLK");
    nameKeyword.push_back("OPC_BRAMADD2_CLK");
    nameKeyword.push_back("OPC_SPI_CLK");
    nameKeyword.push_back("OPC_BSPI_CLK");
    nameKeyword.push_back("OPC_STICK");
    nameKeyword.push_back("OPC_IF");
    nameKeyword.push_back("OPC_IFNOT");
    nameKeyword.push_back("OPC_RAMW");
    nameKeyword.push_back("OPC_UOP");
    nameKeyword.push_back("OPC_UOP");
    nameKeyword.push_back("OPC_UOP");
    nameKeyword.push_back("OPC_UOP");
    nameKeyword.push_back("OPC_LTICK");

    bool keyword = true;
    for (unsigned int i=0; i<codeCursor; ++i)
    {
        if (keyword)
        {
            keyword=false;
            if ((codeResult[i]&0x1F) == OPC_JMPSRC_CLK)
            {
                keyword=true;
            }
            file_readableCompiled << nameKeyword[codeResult[i]&0x1F] << std::endl;
        }
        else
        {
            keyword=true;
            switch (codeResult[i-1]&0xE0)
            {
            case RV_BREAD1:
                file_readableCompiled << "- RV_BREAD1";
                break;
            case RV_BREAD2:
                file_readableCompiled << "- RV_BREAD2";
                break;
            case RV_OPRESULT:
                file_readableCompiled << "- RV_OPRESULT";
                break;
            case RV_RAMVALUE:
                file_readableCompiled << "- RV_RAMVALUE";
                break;
            case RV_SPI:
                file_readableCompiled << "- RV_SPI";
                break;
            case RV_EXT_1:
                file_readableCompiled << "- RV_EXT_1";
                break;
            case RV_EXT_2:
                file_readableCompiled << "- RV_EXT_2";
                break;
            default:
                file_readableCompiled << static_cast<unsigned int>(static_cast<unsigned char>(codeResult[i]));
                break;
            }
            file_readableCompiled << std::endl;
        }
    }

    cout << "Saving OK !" << endl << endl;
    file_compiled.close();
    file.close();
    cout << "Job finished, goodbye !" << endl;

    std::this_thread::sleep_for(4s);

    return 0;
    #endif
}
