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

#include "C_stringDecomposer.hpp"
#include "C_error.hpp"

namespace codeg
{

void StringDecomposer::clear()
{
    this->_flags = codeg::StringDecomposerFlags::FLAGS_EMPTY;
    this->_brut.clear();
    this->_cleaned.clear();
    this->_instruction.clear();
    this->_arguments.clear();
}
void StringDecomposer::decompose(std::string str, const codeg::InlinedStaticMacroList& inlinedStaticMacroList, uint8_t lastFlags)
{
    char lastChar = ' ';
    bool ignoringSpace = true;
    bool ignoring = false;
    bool insideSimpleQuotes = false;
    bool insideDoubleQuotes = false;
    bool insideInlinedStaticMacro = false;
    std::string inlinedStaticMacro;

    this->_flags = codeg::StringDecomposerFlags::FLAGS_EMPTY;
    if ( lastFlags & codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING )
    {
        this->_flags |= codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING;
        ignoring = true;
    }

    this->_cleaned.clear();
    this->_arguments.clear();
    this->_instruction.clear();

    this->_cleaned.reserve(str.size());

    for (char c : str)
    {
        if (!insideDoubleQuotes && !insideSimpleQuotes)
        {//Not ignoring comments
            if (lastChar == '#' && c == '[')
            {//Possible begin of multi-line comments
                this->_flags |= codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING;
                ignoring = true;
            }
            else if (c == '#')
            {//Comment
                if (lastChar == ']')
                {//Possible end of multi-line comments
                    if (!ignoring)
                    {
                        throw codeg::SyntaxError("Unexpected end of multi-line comments !");
                    }
                    this->_flags &=~ codeg::StringDecomposerFlags::FLAG_IGNORE_CHAINING;
                    ignoring = false;
                    lastChar = '#';
                    ignoringSpace = true;
                    continue;
                }
                else
                {
                    ignoring = true;
                }
            }

            if (ignoring)
            {
                lastChar = c;
                ignoringSpace = true;
                continue;
            }
        }

        if (c == ' ')
        {//Space
            if (insideInlinedStaticMacro)
            {
                throw codeg::SyntaxError("Can't have space inside a inlined static macro !");
            }

            if ( !ignoringSpace )
            {
                this->_cleaned.push_back(' ');
                ignoringSpace = true;
            }
        }
        else if (c == '@')
        {
            if (insideInlinedStaticMacro)
            {
                insideInlinedStaticMacro = false;
                if (inlinedStaticMacro.empty())
                {
                    throw codeg::SyntaxError("Can't have empty inlined static macro !");
                }

                auto macroOutput = inlinedStaticMacroList.getReplacement(inlinedStaticMacro);
                if (!macroOutput.has_value())
                {
                    throw codeg::SyntaxError("Unknown inlined static macro : "+inlinedStaticMacro);
                }
                this->_cleaned += macroOutput.value();
                inlinedStaticMacro.clear();
            }
            else
            {
                insideInlinedStaticMacro = true;
            }
        }
        else if (c == '\"')
        {
            if (insideInlinedStaticMacro)
            {
                throw codeg::SyntaxError("Can't have quotation marks inside a inlined static macro !");
            }

            this->_cleaned.push_back('\"');

            if (!insideSimpleQuotes)
            {
                insideDoubleQuotes = !insideDoubleQuotes;
            }
            ignoringSpace = false;
        }
        else if (c == '\'')
        {
            if (insideInlinedStaticMacro)
            {
                throw codeg::SyntaxError("Can't have quotation marks inside a inlined static macro !");
            }

            this->_cleaned.push_back('\'');

            insideSimpleQuotes = !insideSimpleQuotes;
            ignoringSpace = false;
        }
        else if ( (c > 0x20) && (c < 0x7F) )
        {//Only certain ASCII char
            if (insideInlinedStaticMacro)
            {
                inlinedStaticMacro.push_back(c);
            }
            else
            {
                this->_cleaned.push_back(c);
            }
            ignoringSpace = false;
        }

        lastChar = c;
    }
    if (this->_cleaned.length() > 0)
    {
        if ( this->_cleaned.back() == ' ' )
        {
            this->_cleaned.pop_back();
        }
    }

    this->_brut = std::move(str);

    if (insideSimpleQuotes || insideDoubleQuotes)
    {
        throw codeg::SyntaxError("char/string quotation marks without an end !");
    }
    if (insideInlinedStaticMacro)
    {
        throw codeg::SyntaxError("inlined static macro declaration without an end !");
    }

    this->_cleaned.shrink_to_fit();
    codeg::SplitKeywords(this->_cleaned, this->_arguments);
    if (!this->_arguments.empty())
    {
        this->_instruction = std::move(this->_arguments.front());
        this->_arguments.erase( this->_arguments.cbegin() );
    }
}

}//end codeg
