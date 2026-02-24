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

#ifndef vsoa_parser_json_mapping_Deserializer_hpp
#define vsoa_parser_json_mapping_Deserializer_hpp

#include "vsoa_dto/parser/json/Utils.hpp"
#include "vsoa_dto/core/parser/Caret.hpp"
#include "vsoa_dto/core/Types.hpp"

#include <vector>

namespace vsoa { namespace parser { namespace json { namespace mapping {

/**
 * Json Deserializer.
 */
class Deserializer {
public:
  typedef vsoa::data::mapping::type::Type Type;
  typedef vsoa::data::mapping::type::BaseObject::Property Property;
  typedef vsoa::data::mapping::type::BaseObject::Properties Properties;

  typedef vsoa::String String;

public:

  /**
 * "'{' - expected"
 */
  static constexpr v_int32 ERROR_CODE_OBJECT_SCOPE_OPEN = 1;

  /**
   * "'}' - expected"
   */
  static constexpr v_int32 ERROR_CODE_OBJECT_SCOPE_CLOSE = 2;

  /**
   * "Unknown field"
   */
  static constexpr v_int32 ERROR_CODE_OBJECT_SCOPE_UNKNOWN_FIELD = 3;

  /**
   * "':' - expected"
   */
  static constexpr v_int32 ERROR_CODE_OBJECT_SCOPE_COLON_MISSING = 4;

  /**
   * "'[' - expected"
   */
  static constexpr v_int32 ERROR_CODE_ARRAY_SCOPE_OPEN = 5;

  /**
   * "']' - expected"
   */
  static constexpr v_int32 ERROR_CODE_ARRAY_SCOPE_CLOSE = 6;

  /**
   * "'true' or 'false' - expected"
   */
  static constexpr v_int32 ERROR_CODE_VALUE_BOOLEAN = 7;

public:

  /**
   * Deserializer config.
   */
  class Config : public vsoa::base::Countable {
  public:
    /**
     * Constructor.
     */
    Config()
    {}
  public:

    /**
     * Create shared Config.
     * @return - `std::shared_ptr` to Config.
     */
    static std::shared_ptr<Config> createShared(){
      return std::make_shared<Config>();
    }

    /**
     * Do not fail if unknown field is found in json.
     * "unknown field" is the one which is not present in DTO object class.
     */
    bool allowUnknownFields = true;

    /**
     * Enable type interpretations.
     */
    std::vector<std::string> enabledInterpretations = {};

  };

public:
  typedef vsoa::Void (*DeserializerMethod)(Deserializer*, parser::Caret&, const Type* const);
private:
  static void skipScope(vsoa::parser::Caret& caret, v_char8 charOpen, v_char8 charClose);
  static void skipString(vsoa::parser::Caret& caret);
  static void skipToken(vsoa::parser::Caret& caret);
  static void skipValue(vsoa::parser::Caret& caret);
private:
  static const Type* guessNumberType(vsoa::parser::Caret& caret);
  static const Type* guessType(vsoa::parser::Caret& caret);
private:

  template<class T>
  static vsoa::Void deserializeInt(Deserializer* deserializer, parser::Caret& caret, const Type* const type){

    (void) deserializer;
    (void) type;

    if(caret.isAtText("null", true)){
      return vsoa::Void(T::Class::getType());
    } else {
      //TODO: shall we handle overflow cases like
      // vsoa::String json = "128";
      // auto value = jsonObjectMapper->readFromString<vsoa::Int8>(json); // UInt8 will overflow to -128
      return T(static_cast<typename T::UnderlyingType>(caret.parseInt()));
    }

  }

  template<class T>
  static vsoa::Void deserializeUInt(Deserializer* deserializer, parser::Caret& caret, const Type* const type){

    (void) deserializer;
    (void) type;

    if(caret.isAtText("null", true)){
      return vsoa::Void(T::Class::getType());
    } else {
      //TODO: shall we handle overflow cases like
      // vsoa::String json = "256";
      // auto value = jsonObjectMapper->readFromString<vsoa::UInt8>(json); // UInt8 will overflow to 0
      return T(static_cast<typename T::UnderlyingType>(caret.parseUnsignedInt()));
    }

  }

  static vsoa::Void deserializeFloat32(Deserializer* deserializer, parser::Caret& caret, const Type* const type);
  static vsoa::Void deserializeFloat64(Deserializer* deserializer, parser::Caret& caret, const Type* const type);
  static vsoa::Void deserializeBoolean(Deserializer* deserializer, parser::Caret& caret, const Type* const type);
  static vsoa::Void deserializeString(Deserializer* deserializer, parser::Caret& caret, const Type* const type);
  static vsoa::Void deserializeAny(Deserializer* deserializer, parser::Caret& caret, const Type* const type);
  static vsoa::Void deserializeEnum(Deserializer* deserializer, parser::Caret& caret, const Type* const type);

  static vsoa::Void deserializeCollection(Deserializer* deserializer, parser::Caret& caret, const Type* type);
  static vsoa::Void deserializeMap(Deserializer* deserializer, parser::Caret& caret, const Type* const type);

  static vsoa::Void deserializeObject(Deserializer* deserializer, parser::Caret& caret, const Type* const type);

private:
  std::shared_ptr<Config> m_config;
  std::vector<DeserializerMethod> m_methods;
public:

  /**
   * Constructor.
   * @param config
   */
  Deserializer(const std::shared_ptr<Config>& config = std::make_shared<Config>());

  /**
   * Set deserializer method for type.
   * @param classId - &id:vsoa::data::mapping::type::ClassId;.
   * @param method - `typedef vsoa::Void (*DeserializerMethod)(Deserializer*, parser::Caret&, const Type* const)`.
   */
  void setDeserializerMethod(const data::mapping::type::ClassId& classId, DeserializerMethod method);

  /**
   * Deserialize text.
   * @param caret - &id:vsoa::parser::Caret;.
   * @param type - &id:vsoa::data::mapping::type::Type;
   * @return - `vsoa::Void` over deserialized object.
   */
  vsoa::Void deserialize(parser::Caret& caret, const Type* const type);

  /**
   * Get deserializer config.
   * @return
   */
  const std::shared_ptr<Config>& getConfig();

};

}}}}

#endif /* vsoa_parser_json_mapping_Deserializer_hpp */
