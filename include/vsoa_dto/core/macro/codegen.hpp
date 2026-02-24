/***************************************************************************
 *
 * Project VSOA PLATFORM SDK
 *
 *
 * Copyright  SylixOS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef vsoa_macro_codegen_hpp
#define vsoa_macro_codegen_hpp

#include "./basic.hpp"

#define VSOA_CODEGEN_DEFINE_ApiClient "oatpp/codegen/ApiClient_define.hpp"
#define VSOA_CODEGEN_UNDEF_ApiClient "oatpp/codegen/ApiClient_undef.hpp"

#define VSOA_CODEGEN_DEFINE_ApiController "oatpp/codegen/ApiController_define.hpp"
#define VSOA_CODEGEN_UNDEF_ApiController "oatpp/codegen/ApiController_undef.hpp"

#define VSOA_CODEGEN_DEFINE_DbClient "oatpp/codegen/DbClient_define.hpp"
#define VSOA_CODEGEN_UNDEF_DbClient "oatpp/codegen/DbClient_undef.hpp"

#define VSOA_CODEGEN_DEFINE_DTO "vsoa_dto/codegen/DTO_define.hpp"
#define VSOA_CODEGEN_UNDEF_DTO "vsoa_dto/codegen/DTO_undef.hpp"

#define VSOA_CODEGEN_BEGIN(NAME) VSOA_MACRO_EXPAND(VSOA_CODEGEN_DEFINE_ ## NAME)
#define VSOA_CODEGEN_END(NAME) VSOA_MACRO_EXPAND(VSOA_CODEGEN_UNDEF_ ## NAME)

#endif /* vsoa_macro_codegen_hpp */
