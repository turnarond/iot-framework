/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: status.hpp .
*
* Date: 2024-02-22
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_STATUS_H_
#define _VSOA_SDK_STATUS_H_

#include "vsoa_dto/core/Types.hpp"

namespace vsoa {
    /**
    * Http status.
    */
    class Status {
    public:

    /**
    * Success.
    */
    static const Status CODE_0;

    /**
    * Invalid password.
    */
    static const Status CODE_1;// Continue

    /**
    * Invalid arguments.
    */
    static const Status CODE_2;

    /**
    * Invalid url.
    */
    static const Status CODE_3;

    /**
    * No respond.
    */
    static const Status CODE_4;

    /**
    * No permissions.
    */
    static const Status CODE_5;

    /**
    * No memory.
    */
    static const Status CODE_6;

    /**
    * Constructor.
    */
    Status()
        : code(0)
        , description(nullptr)
    {}

    /**
    * Constructor.
    * @param pCode - status code.
    * @param pDesc - description.
    */
    Status(v_int32 pCode, const char* pDesc)
        : code(pCode)
        , description(pDesc)
    {}

    /**
    * Status code.
    */
    v_int32 code;

    /**
    * Description.
    */
    const char* description;
    
    bool operator == (const Status& other) const {
        return this->code == other.code;
    }
    
    bool operator != (const Status& other) const {
        return this->code != other.code;
    }
    
    };
} // namespace vsoa

#endif //_VSOA_SDK_STATUS_H_

/*
 * end
 */