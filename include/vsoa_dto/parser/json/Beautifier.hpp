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

#ifndef vsoa_parser_json_Beautifier_hpp
#define vsoa_parser_json_Beautifier_hpp

#include "vsoa_dto/core/data/stream/Stream.hpp"

namespace vsoa { namespace parser { namespace json {

/**
 * JSON output stream beautifier.
 */
class Beautifier : public vsoa::data::stream::ConsistentOutputStream {
public:
  typedef vsoa::data::stream::ConsistentOutputStream ConsistentOutputStream;
  typedef vsoa::data::stream::IOMode IOMode;
  typedef vsoa::data::stream::Context Context;
private:
  ConsistentOutputStream* m_outputStream;
  vsoa::String m_indent;
  vsoa::String m_newLine;
private:
  v_int32 m_level;
  bool m_wantIndent;
  bool m_isCharEscaped;
  bool m_isInString;
private:
  void writeIndent(ConsistentOutputStream* outputStream);
public:
  
  /**
   * Constructor.
   * @param outputStream - destination output stream.
   * @param indent
   * @param newLine
   */
  Beautifier(ConsistentOutputStream* outputStream, const vsoa::String& indent, const vsoa::String& newLine);

  /**
   * Write `count` of bytes to stream.
   * @param data - data to write.
   * @param count - number of bytes to write.
   * @param action
   * @return - actual number of bytes written. &id:vsoa::v_io_size;.
   */
  v_io_size write(const void *data, v_buff_size count, async::Action& action) override;

  /**
   * Set stream I/O mode.
   * @throws
   */
  void setOutputStreamIOMode(IOMode ioMode) override;

  /**
   * Get stream I/O mode.
   * @return
   */
  IOMode getOutputStreamIOMode() override;

  /**
   * Get stream context.
   * @return
   */
  Context& getOutputStreamContext() override;

};

}}}

#endif // vsoa_parser_json_Beautifier_hpp
