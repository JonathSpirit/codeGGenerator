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

#ifndef C_INSTRUCTION_H_INCLUDED
#define C_INSTRUCTION_H_INCLUDED

#include "C_stringDecomposer.hpp"
#include <forward_list>
#include <memory>

namespace codeg
{

enum BinaryOpcodes : uint8_t
{
    OPCODE_BWRITE1_CLK    = 0x00,
    OPCODE_BWRITE2_CLK    = 0x01,

    OPCODE_BPCS_CLK       = 0x02,

    OPCODE_OPLEFT_CLK     = 0x03,
    OPCODE_OPRIGHT_CLK    = 0x04,
    OPCODE_OPCHOOSE_CLK   = 0x05,

    OPCODE_PERIPHERAL_CLK = 0x06,

    OPCODE_BJMPSRC1_CLK   = 0x07,
    OPCODE_BJMPSRC2_CLK   = 0x08,
    OPCODE_BJMPSRC3_CLK   = 0x09,
    OPCODE_JMPSRC_CLK     = 0x0A,

    OPCODE_BRAMADD1_CLK   = 0x0B,
    OPCODE_BRAMADD2_CLK   = 0x0C,

    OPCODE_SPI_CLK        = 0x0D,
    OPCODE_BCFG_SPI_CLK   = 0x0E,

    OPCODE_STICK          = 0x0F,

    OPCODE_IF             = 0x10,
    OPCODE_IFNOT          = 0x11,

    OPCODE_RAMW           = 0x12,
    /*
    OPCODE_UOP            = 0x13,
    OPCODE_UOP            = 0x14,
    OPCODE_UOP            = 0x15,
    OPCODE_UOP            = 0x16,
    */
    OPCODE_LTICK          = 0x17
};
extern const char* ReadableStringBinaryOpcodes[];

#define ToReadableOpcode(x) codeg::ReadableStringBinaryOpcodes[ (x&0x1F)>0x17 ? 0x13 : (x&0x1F) ]

struct CompilerData;

class Instruction
{
public:
    Instruction() = default;
    virtual ~Instruction() = default;

    virtual std::string getName() const = 0;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) = 0;
    virtual void compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class InstructionList
{
public:
    using InstructionListType = std::forward_list<std::unique_ptr<codeg::Instruction> >;

    InstructionList() = default;
    ~InstructionList() = default;

    void clear();

    void push(codeg::Instruction* newInstruction);
    codeg::Instruction* get(const std::string& name) const;

private:
    codeg::InstructionList::InstructionListType g_data;
};

/// --------- Instructions ---------

class Instruction_set : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    set             set [string] [string]       Define a new macro with a name [string] by the content of the argument [string].
    **/
public:
    Instruction_set();
    virtual ~Instruction_set();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_unset : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    unset           unset [string]              Undefine an existing macro with a name [string].
    **/
public:
    Instruction_unset();
    virtual ~Instruction_unset();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_var : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    var             var [name] ([name])         Define a new [variable] with a [name].
    **/
public:
    Instruction_var();
    virtual ~Instruction_var();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_label : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    label           label [name] ([value])      Define a new jump label to a position in the code.
    **/
public:
    Instruction_label();
    virtual ~Instruction_label();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_jump : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                                               DESCRIPTION
    jump            jump [name] / [value] / [value] [value] [value]         Jump to a certain label or on a fixed/dynamic address with variables.
    **/
public:
    Instruction_jump();
    virtual ~Instruction_jump();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_restart : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    restart         restart                     Jump to the address 0.
    **/
public:
    Instruction_restart();
    virtual ~Instruction_restart();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_affect : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                                   DESCRIPTION
    affect          [constant] [value]                          Assigns a fixed specified address [constant] to a certain [value].
                    [variable] [value]                          Assigns a [variable] to a certain [value].
                    [name] [constant] [value] ([value]...)      Assigns in a fixed size pool [name] with an fixed address offset [constant] a certain [value].
    **/
public:
    Instruction_affect();
    virtual ~Instruction_affect();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_get : public Instruction
{
    /**
    KEYWORD         ARGUMENTS               DESCRIPTION
    get             [constant]              Get a fixed specified address [constant].
                    [variable]              Get a [variable].
                    [name] [constant]       Get in a fixed size pool [name] with an fixed address offset [constant].
    You can use the result with keyword "_ram".
    **/
public:
    Instruction_get();
    virtual ~Instruction_get();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};


class Instruction_write : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    write           write [bus] [value]         Assigns a [bus] to a certain [value].
    **/
public:
    Instruction_write();
    virtual ~Instruction_write();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_choose : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    choose          choose [target] [value]     Choose a [target] with an identity [value].
    **/
public:
    Instruction_choose();
    virtual ~Instruction_choose();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_do : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    do              do [value] [value] [value]  Do a calcul : opleft operation opright.
    **/
public:
    Instruction_do();
    virtual ~Instruction_do();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_tick : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    tick            tick [string] ([value])     No effect instruction (delay).
    **/
public:
    Instruction_tick();
    virtual ~Instruction_tick();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_brut : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    brut            brut [value], ...           Insert instruction in binary, hexadecimal or decimal form.
    **/
public:
    Instruction_brut();
    virtual ~Instruction_brut();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_function : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    function        function [name]             Creates a function with a certain [name].
    **/
public:
    Instruction_function();
    virtual ~Instruction_function();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_if : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    if              if [value]                  Condition statement.
    **/
public:
    Instruction_if();
    virtual ~Instruction_if();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_else : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    else            else                        The “else” tag is used to specify a code to execute when the condition is false.
    **/
public:
    Instruction_else();
    virtual ~Instruction_else();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_ifnot : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    if_not          if_not [value]              Inverted condition statement.
    **/
public:
    Instruction_ifnot();
    virtual ~Instruction_ifnot();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_end : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    end             end                         End of statement.
    **/
public:
    Instruction_end();
    virtual ~Instruction_end();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_call : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                                           DESCRIPTION
    call            call [name] ([variable] [variable] [variable])      Call a function or definition.
    **/
public:
    Instruction_call();
    virtual ~Instruction_call();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_clock : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    clock           clock [target] ([value])    Sends a specified number of pulses to the [target].
    **/
public:
    Instruction_clock();
    virtual ~Instruction_clock();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_pool : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    pool            pool [name] [value] ([value])   Create or modify a pool.
    **/
public:
    Instruction_pool();
    virtual ~Instruction_pool();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_import : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    import          import [string]             import a another codeG file into the current 'import' call
    **/
public:
    Instruction_import();
    virtual ~Instruction_import();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class Instruction_definition : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    definition      definition [name]           Creates a definition with a certain [name].
    **/
public:
    Instruction_definition();
    virtual ~Instruction_definition();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};
class Instruction_enddef : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    end_def         end_def                     end a definition.
    **/
public:
    Instruction_enddef();
    virtual ~Instruction_enddef();

    virtual std::string getName() const;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data);
    virtual void compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

/*
KEYWORD         ARGUMENTS                   DESCRIPTION
repeat          repeat [variable] [value]   Repeat the code.

Repeats the following code according to the argument [value] and a [variable].

The "end" tag has to be at the end.
*/

}//end codeg

#endif // C_INSTRUCTION_H_INCLUDED
