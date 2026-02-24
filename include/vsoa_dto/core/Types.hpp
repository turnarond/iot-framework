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

#ifndef vsoa_Types_hpp
#define vsoa_Types_hpp

#include "vsoa_dto/core/data/mapping/type/Object.hpp"

namespace vsoa {

  /**
   * &id:vsoa::data::mapping::type::Type;.
   */
  typedef vsoa::data::mapping::type::Type Type;

  /**
   * &id:vsoa::data::mapping::type::ClassId;.
   */
  typedef vsoa::data::mapping::type::ClassId ClassId;

  /**
   * ObjectWrapper.
   */
  template <class T, class Clazz = vsoa::data::mapping::type::__class::Void>
  using ObjectWrapper = vsoa::data::mapping::type::ObjectWrapper<T, Clazz>;

  /**
   * ObjectWrapper over the `void*`.
   */
  typedef vsoa::data::mapping::type::Void Void;

  /**
   * `Any` - container for mapping-enabled types.
   * &id:vsoa::data::mapping::type::Any;
   */
  typedef vsoa::data::mapping::type::Any Any;

  /**
   * Mapping-Enabled String type. &id:vsoa::data::mapping::type::String; <br>
   * For `vsoa::String` methods see `std::string`
   */
  typedef vsoa::data::mapping::type::String String;

  /**
   * Mapping-Enabled 8-bits int. Can hold nullptr value. &id:vsoa::data::mapping::type::Int8;
   */
  typedef vsoa::data::mapping::type::Int8 Int8;

  /**
   * Mapping-Enabled 8-bits unsigned int. Can hold nullptr value. &id:vsoa::data::mapping::type::UInt8;
   */
  typedef vsoa::data::mapping::type::UInt8 UInt8;

  /**
   * Mapping-Enabled 16-bits int. Can hold nullptr value. &id:vsoa::data::mapping::type::Int16;
   */
  typedef vsoa::data::mapping::type::Int16 Int16;

  /**
   * Mapping-Enabled 16-bits unsigned int. Can hold nullptr value. &id:vsoa::data::mapping::type::UInt16;
   */
  typedef vsoa::data::mapping::type::UInt16 UInt16;

  /**
   * Mapping-Enabled 32-bits int. Can hold nullptr value. &id:vsoa::data::mapping::type::Int32;
   */
  typedef vsoa::data::mapping::type::Int32 Int32;

  /**
   * Mapping-Enabled 32-bits unsigned int. Can hold nullptr value. &id:vsoa::data::mapping::type::UInt32;
   */
  typedef vsoa::data::mapping::type::UInt32 UInt32;

  /**
   * Mapping-Enabled 64-bits int. Can hold nullptr value. &id:vsoa::data::mapping::type::Int64;
   */
  typedef vsoa::data::mapping::type::Int64 Int64;

  /**
   * Mapping-Enabled 64-bits unsigned int. Can hold nullptr value. &id:vsoa::data::mapping::type::UInt64;
   */
  typedef vsoa::data::mapping::type::UInt64 UInt64;

  /**
   * Mapping-Enabled 32-bits float. Can hold nullptr value. &id:vsoa::data::mapping::type::Float32;
   */
  typedef vsoa::data::mapping::type::Float32 Float32;

  /**
   * Mapping-Enabled 64-bits float (double). Can hold nullptr value. &id:vsoa::data::mapping::type::Float64;
   */
  typedef vsoa::data::mapping::type::Float64 Float64;

  /**
   * Mapping-Enabled Boolean. Can hold nullptr value. &id:vsoa::data::mapping::type::Boolean;
   */
  typedef vsoa::data::mapping::type::Boolean Boolean;

  /**
   * Base class for all Object-like Mapping-enabled structures. &id:vsoa::data::mapping::type::BaseObject;
   */
  typedef vsoa::data::mapping::type::BaseObject BaseObject;

  /**
   * Base class for all DTO objects. &id:vsoa::data::mapping::type::DTO;
   */
  typedef vsoa::data::mapping::type::DTO DTO;

  /**
   * Mapping-Enabled DTO Object. &id:vsoa::data::mapping::type::DTOWrapper;
   */
  template <class T>
  using Object = vsoa::data::mapping::type::DTOWrapper<T>;

  /**
   * Mapping-Enabled Enum. &id:vsoa::data::mapping::type::Enum;
   */
  template <class T>
  using Enum = vsoa::data::mapping::type::Enum<T>;

  /**
   * Mapping-Enabled Vector. &id:vsoa::data::mapping::type::Vector;
   */
  template <class T>
  using Vector = vsoa::data::mapping::type::Vector<T>;

  /**
   * Abstract Vector.
   */
  typedef vsoa::data::mapping::type::AbstractVector AbstractVector;

  /**
   * Mapping-Enabled List. &id:vsoa::data::mapping::type::List;
   */
  template <class T>
  using List = vsoa::data::mapping::type::List<T>;

  /**
   * Abstract List.
   */
  typedef vsoa::data::mapping::type::AbstractList AbstractList;

  /**
   * Mapping-Enabled UnorderedSet. &id:vsoa::data::mapping::type::UnorderedSet;
   */
  template <class T>
  using UnorderedSet = vsoa::data::mapping::type::UnorderedSet<T>;

  /**
   * Abstract UnorderedSet.
   */
  typedef vsoa::data::mapping::type::AbstractUnorderedSet AbstractUnorderedSet;

  /**
   * Mapping-Enabled PairList<Key, Value>. &id:vsoa::data::mapping::type::PairList;
   */
  template <class Key, class Value>
  using PairList = vsoa::data::mapping::type::PairList<Key, Value>;

  /**
   * Mapping-Enabled PairList<String, Value>. &id:vsoa::data::mapping::type::PairList;
   */
  template <class Value>
  using Fields = vsoa::PairList<String, Value>;

  /**
   * Abstract Fields
   */
  typedef Fields<vsoa::Void> AbstractFields;

  /**
   * Mapping-Enabled UnorderedMap<Key, Value>. &id:vsoa::data::mapping::type::UnorderedMap;.
   */
  template <class Key, class Value>
  using UnorderedMap = vsoa::data::mapping::type::UnorderedMap<Key, Value>;

  /**
   * Mapping-Enabled UnorderedMap<String, Value>. &id:vsoa::data::mapping::type::UnorderedMap;.
   */
  template <class Value>
  using UnorderedFields = vsoa::UnorderedMap<String, Value>;

  /**
   * Abstract UnorderedFields
   */
  typedef UnorderedFields<vsoa::Void> AbstractUnorderedFields;
  
}

#endif /* vsoa_Types_hpp */
