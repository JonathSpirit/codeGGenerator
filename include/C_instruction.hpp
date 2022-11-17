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

#ifndef C_INSTRUCTION_HPP_INCLUDED
#define C_INSTRUCTION_HPP_INCLUDED

#include "C_stringDecomposer.hpp"
#include <vector>
#include <memory>

#define CODEG_BINARYOPCODES_MASK 0x1F

namespace codeg
{

struct ArgumentsRange
{
    ArgumentsRange(unsigned int min, signed int max) :
            _min(min),
            _max(max)
    {}
    ArgumentsRange(unsigned int minMax) :
            _min(minMax),
            _max(static_cast<signed int>(minMax))
    {}

    unsigned int _min;
    signed int _max;

    [[nodiscard]] inline bool check(std::size_t size) const
    {
        return size >= this->_min && ((this->_max < 0) || (size <= static_cast<std::size_t>(this->_max)));
    }
    [[nodiscard]] std::string toString() const
    {
        if (static_cast<signed int>(this->_min) == this->_max)
        {
            return std::to_string(this->_min);
        }

        if (this->_max < 0)
        {
            auto stringSize = std::snprintf(nullptr, 0, ">= %u", this->_min) + 1;

            std::unique_ptr<char[]> data{new char[stringSize]};
            std::snprintf(data.get(), stringSize, ">= %u", this->_min);

            std::string output(data.get(), stringSize-1);
            return output;
        }

        auto stringSize = std::snprintf(nullptr, 0, "%u to %d", this->_min, this->_max) + 1;

        std::unique_ptr<char[]> data{new char[stringSize]};
        std::snprintf(data.get(), stringSize, "%u to %d", this->_min, this->_max);

        std::string output(data.get(), stringSize-1);
        return output;
    }
};

struct ArgumentsSize
{
    std::vector<codeg::ArgumentsRange> _ranges;

    [[nodiscard]] inline bool check(std::size_t size) const
    {
        for (const auto& range : this->_ranges)
        {
            if (range.check(size))
            {
                return true;
            }
        }
        return false;
    }
    [[nodiscard]] std::string toString() const
    {
        std::string output;
        for (std::size_t i=0; i<this->_ranges.size(); ++i)
        {
            output += this->_ranges[i].toString();
            if (i != this->_ranges.size()-1)
            {
                output += " or ";
            }
        }
        return output;
    }
};

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

const char* OpcodeToString(uint8_t opcode);

struct CompilerData;

class Instruction
{
public:
    Instruction() = default;
    virtual ~Instruction() = default;

    [[nodiscard]] virtual std::string getName() const = 0;

    [[nodiscard]] virtual codeg::ArgumentsSize getArgumentsSize() const = 0;

    virtual void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) = 0;
    virtual void compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data);
};

class InstructionList
{
public:
    using InstructionListType = std::vector<std::unique_ptr<codeg::Instruction> >;

    InstructionList();
    ~InstructionList() = default;

    void clear();

    void push(std::unique_ptr<codeg::Instruction>&& newInstruction);
    [[nodiscard]] codeg::Instruction* get(const std::string& name) const;

private:
    codeg::InstructionList::InstructionListType g_data;
};

/// --------- Instructions ---------

class Instruction_set : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    set             set [string] [string]       Define a new macro [string] by the content of the argument [string].
    **/
public:
    Instruction_set() = default;
    ~Instruction_set() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{2,2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_unset : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    unset           unset [string]              Remove an existing macro [string].
    **/
public:
    Instruction_unset() = default;
    ~Instruction_unset() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_var : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    var             var [variable]              Declare a new [variable] in the default/wanted pool.
                    var [variable] [constant]   Declare a new [variable] in the default/wanted pool, with a wanted size [constant].
    **/
public:
    Instruction_var() = default;
    ~Instruction_var() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1,2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_label : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    label           label [name]                Define a new jump label [name] to the actual position in the code.
                    label [name] [constant]     Define a new jump label [name] to custom address position [constant].
    **/
public:
    Instruction_label() = default;
    ~Instruction_label() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1,2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_jump : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                            DESCRIPTION
    jump            jump [name]                          Jump to a certain label [name].
                    jump [constant]                      Jump to a custom address position [constant].
                    jump [value] [value] [value]         Jump to a dynamic address position 3x[value] (MSB first) .
    **/
public:
    Instruction_jump() = default;
    ~Instruction_jump() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}, {3}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_restart : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    restart         restart                     Jump to the address 0.
    **/
public:
    Instruction_restart() = default;
    ~Instruction_restart() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{0}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_affect : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                                   DESCRIPTION
    affect          [constant] [value]                          Assigns a fixed specified address [constant] to a certain [value].
                    [variable] [value] ([constant])             Assigns a [variable] to a certain [value] with an fixed address offset [constant].
                    [name] [constant] [value] ([value]...)      Assigns in a fixed size pool [name] with an fixed address offset [constant] a certain [value] (or multiple).
                    [value]                                     Assigns without setting the address a certain [value].
    **/
public:
    Instruction_affect() = default;
    ~Instruction_affect() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{2, -1}, {3, -1}, {1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_get : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                 DESCRIPTION
    get             [constant]                Get a fixed specified address [constant].
                    [variable] ([constant])   Get a [variable] with an fixed address offset [constant].
                    [name] [constant]         Get in a fixed size pool [name] with an fixed address offset [constant].
    You can use the result with keyword "_ram".
    **/
public:
    Instruction_get() = default;
    ~Instruction_get() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1,2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};


class Instruction_write : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    write           write [bus] [value]         Assigns a [bus] to a certain [value].
    **/
public:
    Instruction_write() = default;
    ~Instruction_write() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_select : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    select          select [target] [value]     Select a [target] with an identity [value].
    **/
public:
    Instruction_select() = default;
    ~Instruction_select() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_do : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                    DESCRIPTION
    do              do [value] [value] [value]   Do a calculation: opleft [value] operation [value] opright [value].
    do              do [value] [value]           Do a calculation (without selecting operation): opleft [value] opright [value].
    **/
public:
    Instruction_do() = default;
    ~Instruction_do() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{2,3}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_tick : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                    DESCRIPTION
    tick            tick [string]                No effect instruction "long"/"simple" tick delay [string].
                    tick [string] [constant]     No effect instruction "long"/"simple" tick delay [string] copied [constant] times.
    **/
public:
    Instruction_tick() = default;
    ~Instruction_tick() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1,2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_brut : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    brut            brut [constant], ...        Insert instruction in binary, hexadecimal or decimal form.
    **/
public:
    Instruction_brut() = default;
    ~Instruction_brut() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1, -1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_function : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    function        function [name]             Creates a function with a certain [name].
    **/
public:
    Instruction_function() = default;
    ~Instruction_function() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_if : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    if              if [value]                  Condition statement.
    **/
public:
    Instruction_if() = default;
    ~Instruction_if() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_else : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    else            else                        The �else� tag is used to specify a code to execute when the condition is false.
    **/
public:
    Instruction_else() = default;
    ~Instruction_else() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{0}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_ifnot : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    if_not          if_not [value]              Inverted condition statement.
    **/
public:
    Instruction_ifnot() = default;
    ~Instruction_ifnot() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_end : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    end             end                         End of statement.
    **/
public:
    Instruction_end() = default;
    ~Instruction_end() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{0}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_call : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                                           DESCRIPTION
    call            call [name]                                         Call a function or definition.
                    call [name] [variable] [variable] [variable]        Call a function and store in 3 separate [variable] MSB first the return address.
    **/
public:
    Instruction_call() = default;
    ~Instruction_call() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}, {4}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_clock : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    clock           clock [target]              Sends a clock pulse to the [target].
                    clock [target] [constant]   Sends a specified number of pulses [constant] to the [target].
    **/
public:
    Instruction_clock() = default;
    ~Instruction_clock() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1,2}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_pool : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                             DESCRIPTION
    pool            pool [name] [constant]                Create or modify a pool [name] with a certain size [constant] (0 for a dynamic size).
                    pool [name] [constant] [constant]     Create or modify a pool [name] with a certain size [constant] (0 for a dynamic size) and a start address [constant].
    **/
public:
    Instruction_pool() = default;
    ~Instruction_pool() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{2,3}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_import : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    import          import [string]             import another codeG file into the current line to be compiled.
    **/
public:
    Instruction_import() = default;
    ~Instruction_import() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

class Instruction_definition : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    definition      definition [name]           Creates a definition with a certain [name].
    **/
public:
    Instruction_definition() = default;
    ~Instruction_definition() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{1}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};
class Instruction_enddef : public Instruction
{
    /**
    KEYWORD         ARGUMENTS                   DESCRIPTION
    end_def         end_def                     end a definition.
    **/
public:
    Instruction_enddef() = default;
    ~Instruction_enddef() override = default;

    [[nodiscard]] std::string getName() const override;

    [[nodiscard]] codeg::ArgumentsSize getArgumentsSize() const override {return {{{0}}};}

    void compile(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
    void compileDefinition(const codeg::StringDecomposer& input, codeg::CompilerData& data) override;
};

}//end codeg

#endif // C_INSTRUCTION_HPP_INCLUDED
