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

#ifndef C_MACRO_H_INCLUDED
#define C_MACRO_H_INCLUDED

#include <string>
#include <map>

namespace codeg
{

typedef std::map<std::string, std::string> MacroList;

bool MacroReplace(const codeg::MacroList& macroList, std::string& str);
void MacroSet(codeg::MacroList& macroList, const std::string& key, const std::string& str);
bool MacroRemove(codeg::MacroList& macroList, const std::string& key);
bool MacroCheck(const codeg::MacroList& macroList, const std::string& key);

}//end codeg

#endif // C_MACRO_H_INCLUDED
