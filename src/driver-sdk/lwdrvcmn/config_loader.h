/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: config_loader.h .
*
* Date: 2026-02-11
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "lwdrvcmn.h"
#include <string>
#include <vector>

class ConfigLoader {
private:
    std::string config_path_;
    std::string db_path_;

public:
    ConfigLoader(const std::string& config_path, const std::string& db_path);
    ~ConfigLoader();

    /**
     * 加载配置
     * 优先读取xml文件，如果xml文件不存在则读取数据库，并生成xml文件
     * @return 加载是否成功
     */
    bool LoadConfig();

    /**
     * 获取驱动列表
     * @return 驱动列表
     */
    std::vector<LWDRIVER*> GetDrivers();

    /**
     * 保存配置到xml文件
     * @return 保存是否成功
     */
    bool SaveConfigToXml();

    /**
     * 从数据库加载配置
     * @return 加载是否成功
     */
    bool LoadConfigFromDb();

    /**
     * 从xml文件加载配置
     * @return 加载是否成功
     */
    bool LoadConfigFromXml();

private:
    std::vector<LWDRIVER*> drivers_;

    /**
     * 解析连接类型
     * @param conn_type_str 连接类型字符串
     * @return 连接类型整数
     */
    int ParseConnType(const std::string& conn_type_str);

    /**
     * 解析数据类型
     * @param data_type_str 数据类型字符串
     * @return 数据类型整数
     */
    int ParseDataType(const std::string& data_type_str);

    /**
     * 创建LWDRIVER实例
     * @param name 驱动名称
     * @param type 驱动类型
     * @param version 驱动版本
     * @param description 驱动描述
     * @param param1_name 参数1名称
     * @param param1_desc 参数1描述
     * @param param2_name 参数2名称
     * @param param2_desc 参数2描述
     * @param param3_name 参数3名称
     * @param param3_desc 参数3描述
     * @param param4_name 参数4名称
     * @param param4_desc 参数4描述
     * @return LWDRIVER实例
     */
    LWDRIVER* CreateDriver(const std::string& name, int type, const std::string& version, 
                          const std::string& description, const std::string& param1_name, 
                          const std::string& param1_desc, const std::string& param2_name, 
                          const std::string& param2_desc, const std::string& param3_name, 
                          const std::string& param3_desc, const std::string& param4_name, 
                          const std::string& param4_desc);

    /**
     * 创建LWDEVICE实例
     * @param name 设备名称
     * @param desc 设备描述
     * @param conn_type 连接类型
     * @param conn_param 连接参数
     * @param param1 参数1
     * @param param2 参数2
     * @param param3 参数3
     * @param param4 参数4
     * @return LWDEVICE实例
     */
    LWDEVICE* CreateDevice(const std::string& name, const std::string& desc, int conn_type, 
                          const std::string& conn_param, const std::string& param1, 
                          const std::string& param2, const std::string& param3, 
                          const std::string& param4);

    /**
     * 创建LWTAG实例
     * @param name 点位名称
     * @param address 点位地址
     * @param data_type 数据类型
     * @param point_type 点位类型
     * @param transfer_type 转换类型
     * @param polling_interval 轮询间隔
     * @param linear_raw_min 线性原始最小值
     * @param linear_raw_max 线性原始最大值
     * @param linear_eng_min 线性工程最小值
     * @param linear_eng_max 线性工程最大值
     * @param advanced_algo_lib 高级算法库
     * @param advanced_param1 高级参数1
     * @param advanced_param2 高级参数2
     * @param advanced_param3 高级参数3
     * @param advanced_param4 高级参数4
     * @param enable_control 是否启用控制
     * @param enable_history 是否启用历史
     * @return LWTAG实例
     */
    LWTAG* CreateTag(const std::string& name, const std::string& address, int data_type, 
                    int point_type, int transfer_type, int polling_interval, 
                    double linear_raw_min, double linear_raw_max, 
                    double linear_eng_min, double linear_eng_max, 
                    const std::string& advanced_algo_lib, const std::string& advanced_param1, 
                    const std::string& advanced_param2, const std::string& advanced_param3, 
                    const std::string& advanced_param4, bool enable_control, 
                    bool enable_history);

    /**
     * 释放驱动列表
     */
    void FreeDrivers();
};
