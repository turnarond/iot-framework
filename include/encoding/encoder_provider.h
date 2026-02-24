/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: encoder_provider.h .
*
* Date: 2024-05-10
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _ENCODER_PROVIDER_H_
#define _ENCODER_PROVIDER_H_

#include <string>

namespace encoding {

class EncoderProvider {
public:
    virtual ~EncoderProvider() = default;

public:
    virtual int InitEncoderProvider(const void *descriptor) = 0;
    virtual int WriteToPack(const void *obj, std::string &out) = 0;
    virtual int ReadFromPack(const void *obj, size_t len, std::string &out) = 0;
    virtual void UninitEncoderProvider(void) = 0;
} ;
} // namespace encoding

#endif // _ENCODER_PROVIDER_H_

/*
 * end
 */