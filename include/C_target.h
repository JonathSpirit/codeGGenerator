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

#ifndef C_TARGET_H_INCLUDED
#define C_TARGET_H_INCLUDED

#include <string>

namespace codeg
{

enum TargetType
{
    TARGET_NULL = 0,

    TARGET_PERIPHERAL,
    TARGET_OPERATION
};

codeg::TargetType ProcessTarget(const std::string& strTarget);

}//end codeg

#endif // C_TARGET_H_INCLUDED
