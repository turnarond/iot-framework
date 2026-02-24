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

#ifndef vsoa_data_mapping_ObjectMapper_hpp
#define vsoa_data_mapping_ObjectMapper_hpp

#include "type/Object.hpp"
#include "type/Type.hpp"

#include "vsoa_dto/core/data/stream/Stream.hpp"

#include "vsoa_dto/core/parser/Caret.hpp"
#include "vsoa_dto/core/parser/ParsingError.hpp"

namespace vsoa { namespace data { namespace mapping {

/**
 * Abstract ObjectMapper class.
 */
class ObjectMapper {
public:

  /**
   * Metadata for ObjectMapper.
   */
  class Info {
  public:

    /**
     * Constructor.
     * @param _http_content_type
     */
    Info(const char* _http_content_type)
      : http_content_type(_http_content_type)
    {}

    /**
     * Value for Content-Type http header when DTO is serialized via specified ObjectMapper.
     */
    const char* const http_content_type;

  };
private:
  Info m_info;
public:

  /**
   * Constructor.
   * @param info - Metadata for ObjectMapper.
   */
  ObjectMapper(const Info& info);

  /**
   * Get ObjectMapper metadata.
   * @return - ObjectMapper metadata.
   */
  const Info& getInfo() const;

  /**
   * Serialize object to stream. Implement this method.
   * @param stream - &id:vsoa::data::stream::ConsistentOutputStream; to serialize object to.
   * @param variant - Object to serialize.
   */
  virtual void write(data::stream::ConsistentOutputStream* stream, const type::Void& variant) const = 0;

  /**
   * Deserialize object. Implement this method.
   * @param caret - &id:vsoa::parser::Caret; over serialized buffer.
   * @param type - pointer to object type. See &id:vsoa::data::mapping::type::Type;.
   * @return - deserialized object wrapped in &id:vsoa::Void;.
   */
  virtual mapping::type::Void read(vsoa::parser::Caret& caret, const mapping::type::Type* const type) const = 0;

  /**
   * Serialize object to String.
   * @param variant - Object to serialize.
   * @return - serialized object as &id:vsoa::String;.
   */
  vsoa::String writeToString(const type::Void& variant) const;

  /**
   * Deserialize object.
   * If nullptr is returned - check caret.getError()
   * @tparam Wrapper - ObjectWrapper type.
   * @param caret - &id:vsoa::parser::Caret; over serialized buffer.
   * @return - deserialized Object.
   * @throws - depends on implementation.
   */
  template<class Wrapper>
  Wrapper readFromCaret(vsoa::parser::Caret& caret) const {
    auto type = Wrapper::Class::getType();
    return read(caret, type).template cast<Wrapper>();
  }

  /**
   * Deserialize object.
   * @tparam Wrapper - ObjectWrapper type.
   * @param str - serialized data.
   * @return - deserialized Object.
   * @throws - &id:vsoa::parser::ParsingError;
   * @throws - depends on implementation.
   */
  template<class Wrapper>
  Wrapper readFromString(const vsoa::String& str) const {
    auto type = Wrapper::Class::getType();
    vsoa::parser::Caret caret(str);
    auto result = read(caret, type).template cast<Wrapper>();
    if(caret.hasError()) {
      throw vsoa::parser::ParsingError(caret.getErrorMessage(), caret.getErrorCode(), caret.getPosition());
    }
    return result;
  }
  
};
  
}}}

#endif /* vsoa_data_mapping_ObjectMapper_hpp */
