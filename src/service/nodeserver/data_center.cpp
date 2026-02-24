/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: data_center.cpp .
*
* Date: 2025-07-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "data_center.h"
#include "rtdb.hpp"
#include <chrono>
#include <lwlog/lwlog.h>

extern CLWLog g_logger;
DataCenter DataCenter::instance_;

DataCenter* DataCenter::GetInstance()
{
    return &instance_;
}

DataCenter::DataCenter()
{
    // 默认分片数由 RTDB 构造时自动计算 (num_cores * 2)
    rtdb_ = std::make_unique<nodeserver::RTDB>(0);
}

DataCenter::~DataCenter() {}

size_t DataCenter::Size() const
{
    return rtdb_->size();
}

bool DataCenter::HealthCheck(std::string* outReason) const
{
    return rtdb_->healthCheck(outReason);
}

nodeserver::RTDB* DataCenter::GetRTDB()
{
    return rtdb_.get();
}