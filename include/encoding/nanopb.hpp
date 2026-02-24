/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: nanopb.hpp .
*
* Date: 2024-05-10
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _NANOPB_HPP_
#define _NANOPB_HPP_

#include "encoder_provider.h"
#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"

namespace encoding {

class NanopbUtil : public EncoderProvider
{
public:
    virtual int InitEncoderProvider(const void *descriptor);
    int WriteToPack(const void *obj, std::string &out);
    int ReadFromPack(const void *obj, size_t len, std::string &out);
    void UninitEncoderProvider(void);

private:
    pb_msgdesc_t *desc;

public:
    NanopbUtil() = default;
    ~NanopbUtil() = default;
} ;

} // encoding

#endif // _NANOPB_HPP_

/*
 * end
 */