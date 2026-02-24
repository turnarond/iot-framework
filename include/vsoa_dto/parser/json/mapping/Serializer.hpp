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

#ifndef vsoa_parser_json_mapping_Serializer_hpp
#define vsoa_parser_json_mapping_Serializer_hpp

#include "vsoa_dto/parser/json/Utils.hpp"
#include "vsoa_dto/parser/json/Beautifier.hpp"
#include "vsoa_dto/core/Types.hpp"
#include <vector>

namespace vsoa { namespace parser { namespace json { namespace mapping {

/**
 * Json Serializer.
 */
class Serializer {
public:
  typedef vsoa::data::mapping::type::Type Type;
  typedef vsoa::data::mapping::type::BaseObject::Property Property;
  typedef vsoa::data::mapping::type::BaseObject::Properties Properties;

  typedef vsoa::String String;
public:
  /**
   * Serializer config.
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
     * Create shared config.
     * @return - `std::shared_ptr` to Config.
     */
    static std::shared_ptr<Config> createShared(){
      return std::make_shared<Config>();
    }

    /**
     * Include fields with value == nullptr into serialized json.
     * Field will still be included when field-info `required` is set to true and &id:alwaysIncludeRequired is set to true.
     */
    bool includeNullFields = true;

    /**
     * Always include required fields (set in in DTO_FIELD_INFO) even if they are `value == nullptr`
     */
    bool alwaysIncludeRequired = false;

    /**
     * Always include array or map elements, even if their value is `nullptr`.
     */
    bool alwaysIncludeNullCollectionElements = false;

    /**
     * If `true` - insert string `"<unknown-type>"` in json field value in case unknown field found.
     * Fail if `false`.
     * Known types for this serializer are:<br>
     * (String, Int8, Int16, Int32, Int64, Float32, Float64, Boolean, DTOs, List, Fields).
     */
    bool throwOnUnknownTypes = true;

    /**
     * Use JSON Beautifier.
     */
    bool useBeautifier = false;

    /**
     * Beautifier Indent.
     */
    vsoa::String beautifierIndent = "  ";

    /**
     * Beautifier new line.
     */
    vsoa::String beautifierNewLine = "\n";

    /**
     * Enable type interpretations.
     */
    std::vector<std::string> enabledInterpretations = {};

    /**
     * Escape flags.
     */
    v_uint32 escapeFlags = json::Utils::FLAG_ESCAPE_ALL;

  };
public:
  typedef void (*SerializerMethod)(Serializer*,
                                   data::stream::ConsistentOutputStream*,
                                   const vsoa::Void&);
private:

  template<class T>
  static void serializePrimitive(Serializer* serializer,
                                 data::stream::ConsistentOutputStream* stream,
                                 const vsoa::Void& polymorph){
    (void) serializer;

    if(polymorph){
      stream->writeAsString(* static_cast<typename T::ObjectType*>(polymorph.get()));
    } else {
      stream->writeSimple("null", 4);
    }
  }
  
  static void serializeString(vsoa::data::stream::ConsistentOutputStream* stream,
                              const char* data,
                              v_buff_size size,
                              v_uint32 escapeFlags);

  static void serializeString(Serializer* serializer,
                              data::stream::ConsistentOutputStream* stream,
                              const vsoa::Void& polymorph);

  static void serializeAny(Serializer* serializer,
                           data::stream::ConsistentOutputStream* stream,
                           const vsoa::Void& polymorph);

  static void serializeEnum(Serializer* serializer,
                            data::stream::ConsistentOutputStream* stream,
                            const vsoa::Void& polymorph);

  static void serializeCollection(Serializer* serializer,
                                  data::stream::ConsistentOutputStream* stream,
                                  const vsoa::Void& polymorph);

  static void serializeMap(Serializer* serializer,
                           data::stream::ConsistentOutputStream* stream,
                           const vsoa::Void& polymorph);

  static void serializeObject(Serializer* serializer,
                              data::stream::ConsistentOutputStream* stream,
                              const vsoa::Void& polymorph);

  void serialize(data::stream::ConsistentOutputStream* stream, const vsoa::Void& polymorph);

private:
  std::shared_ptr<Config> m_config;
  std::vector<SerializerMethod> m_methods;
public:

  /**
   * Constructor.
   * @param config - serializer config.
   */
  Serializer(const std::shared_ptr<Config>& config = std::make_shared<Config>());

  /**
   * Set serializer method for type.
   * @param classId - &id:vsoa::data::mapping::type::ClassId;.
   * @param method - `typedef void (*SerializerMethod)(Serializer*, data::stream::ConsistentOutputStream*, const vsoa::Void&)`.
   */
  void setSerializerMethod(const data::mapping::type::ClassId& classId, SerializerMethod method);

  /**
   * Serialize object to stream.
   * @param stream - &id:vsoa::data::stream::ConsistentOutputStream;.
   * @param polymorph - DTO as &id:vsoa::Void;.
   */
  void serializeToStream(data::stream::ConsistentOutputStream* stream, const vsoa::Void& polymorph);

  /**
   * Get serializer config.
   * @return
   */
  const std::shared_ptr<Config>& getConfig();

};

}}}}

#endif /* vsoa_parser_json_mapping_Serializer_hpp */
