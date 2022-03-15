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

#ifndef C_STRING_HPP_INCLUDED
#define C_STRING_HPP_INCLUDED

#include <string>
#include <vector>

namespace codeg
{

std::size_t Split(const std::string& str, std::vector<std::string>& buff, char delimiter);
std::size_t SplitKeywords(const std::string& str, std::vector<std::string>& buff);

std::string ValueToHex(uint32_t val, unsigned int hexSize=8, bool removeExtraZero=false, bool removePrefix=false);

}//end codeg

#endif // C_STRING_HPP_INCLUDED
