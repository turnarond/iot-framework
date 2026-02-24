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

#ifndef vsoa_parser_json_mapping_ObjectMapper_hpp
#define vsoa_parser_json_mapping_ObjectMapper_hpp

#include "./Serializer.hpp"
#include "./Deserializer.hpp"

#include "vsoa_dto/core/data/mapping/ObjectMapper.hpp"

namespace vsoa { namespace parser { namespace json { namespace mapping {

/**
 * Json ObjectMapper. Serialized/Deserializes vsoa DTO objects to/from JSON.
 * Extends &id:vsoa::base::Countable;, &id:vsoa::data::mapping::ObjectMapper;.
 */
class ObjectMapper : public vsoa::base::Countable, public vsoa::data::mapping::ObjectMapper {
private:
  static Info& getMapperInfo() {
    static Info info("application/json");
    return info;
  }
private:
  std::shared_ptr<Serializer> m_serializer;
  std::shared_ptr<Deserializer> m_deserializer;
public:
  /**
   * Constructor.
   * @param serializerConfig - &id:vsoa::parser::json::mapping::Serializer::Config;.
   * @param deserializerConfig - &id:vsoa::parser::json::mapping::Deserializer::Config;.
   */
  ObjectMapper(const std::shared_ptr<Serializer::Config>& serializerConfig,
               const std::shared_ptr<Deserializer::Config>& deserializerConfig);

  /**
   * Constructor.
   * @param serializer
   * @param deserializer
   */
  ObjectMapper(const std::shared_ptr<Serializer>& serializer = std::make_shared<Serializer>(),
               const std::shared_ptr<Deserializer>& deserializer = std::make_shared<Deserializer>());
public:

  /**
   * Create shared ObjectMapper.
   * @param serializerConfig - &id:vsoa::parser::json::mapping::Serializer::Config;.
   * @param deserializerConfig - &id:vsoa::parser::json::mapping::Deserializer::Config;.
   * @return - `std::shared_ptr` to ObjectMapper.
   */
  static std::shared_ptr<ObjectMapper>
  createShared(const std::shared_ptr<Serializer::Config>& serializerConfig,
               const std::shared_ptr<Deserializer::Config>& deserializerConfig);

  /**
   * Create shared ObjectMapper.
   * @param serializer
   * @param deserializer
   * @return
   */
  static std::shared_ptr<ObjectMapper>
  createShared(const std::shared_ptr<Serializer>& serializer = std::make_shared<Serializer>(),
               const std::shared_ptr<Deserializer>& deserializer = std::make_shared<Deserializer>());

  /**
   * Implementation of &id:vsoa::data::mapping::ObjectMapper::write;.
   * @param stream - stream to write serializerd data to &id:vsoa::data::stream::ConsistentOutputStream;.
   * @param variant - object to serialize &id:vsoa::Void;.
   */
  void write(data::stream::ConsistentOutputStream* stream, const vsoa::Void& variant) const override;

  /**
   * Implementation of &id:vsoa::data::mapping::ObjectMapper::read;.
   * @param caret - &id:vsoa::parser::Caret;.
   * @param type - type of resultant object &id:vsoa::data::mapping::type::Type;.
   * @return - &id:vsoa::Void; holding resultant object.
   */
  vsoa::Void read(vsoa::parser::Caret& caret, const vsoa::data::mapping::type::Type* const type) const override;


  /**
   * Get serializer.
   * @return
   */
  std::shared_ptr<Serializer> getSerializer();

  /**
   * Get deserializer.
   * @return
   */
  std::shared_ptr<Deserializer> getDeserializer();
  
};
  
}}}}

#endif /* vsoa_parser_json_mapping_ObjectMapper_hpp */
