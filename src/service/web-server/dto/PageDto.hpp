/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-27 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/PageDto.hpp
 * @Description: 分页DTO定义，用于分页查询结果的传输和展示
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef CRUD_PAGEDTO_HPP
#define CRUD_PAGEDTO_HPP

#include "vsoa_dto/core/macro/codegen.hpp"
#include "vsoa_dto/core/Types.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

template<class T>
class PageDto : public vsoa::DTO {

  DTO_INIT(PageDto, DTO)

  DTO_FIELD(vsoa::UInt32, page, "page"); ///< 当前页码，从1开始计算
  DTO_FIELD(vsoa::UInt32, size, "size");  ///< 每页数量
  DTO_FIELD(vsoa::UInt32, pages, "pages");  ///< 总页数
  DTO_FIELD(vsoa::UInt32, counts, "counts"); ///< 总数据项数量
  DTO_FIELD(vsoa::Vector<T>, items, "items"); ///< 分页数据项列表
};

#include VSOA_CODEGEN_END(DTO)

#endif
