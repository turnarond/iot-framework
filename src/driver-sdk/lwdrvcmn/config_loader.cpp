/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: config_loader.cpp .
*
* Date: 2026-02-11
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "config_loader.h"
#include "lwlog/lwlog.h"
#include <fstream>
#include <sstream>
#include <sqlite3.h>
#include <cstring>

extern CLWLog g_logger;

ConfigLoader::ConfigLoader(const std::string& config_path, const std::string& db_path)
    : config_path_(config_path), db_path_(db_path)
{}

ConfigLoader::~ConfigLoader()
{
    FreeDrivers();
}

bool ConfigLoader::LoadConfig()
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Loading config from %s", config_path_.c_str());
    
    // 优先从xml文件加载配置
    if (LoadConfigFromXml()) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Config loaded from xml file successfully");
        return true;
    }
    
    // 如果xml文件不存在或加载失败，则从数据库加载配置
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Xml file not found or failed to load, loading from database");
    if (LoadConfigFromDb()) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Config loaded from database successfully");
        
        // 从数据库加载成功后，生成xml文件
        if (SaveConfigToXml()) {
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Config saved to xml file successfully");
        } else {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "Failed to save config to xml file");
        }
        
        return true;
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load config from both xml and database");
    return false;
}

std::vector<LWDRIVER*> ConfigLoader::GetDrivers()
{
    return drivers_;
}

bool ConfigLoader::SaveConfigToXml()
{
    try {
        std::ofstream xml_file(config_path_);
        if (!xml_file.is_open()) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to open xml file for writing: %s", config_path_.c_str());
            return false;
        }
        
        xml_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        xml_file << "<drivers>" << std::endl;
        
        for (auto driver : drivers_) {
            xml_file << "  <driver>" << std::endl;
            xml_file << "    <name>" << (driver->name ? driver->name : "") << "</name>" << std::endl;
            xml_file << "    <type>" << driver->type << "</type>" << std::endl;
            xml_file << "    <version>" << (driver->version ? driver->version : "") << "</version>" << std::endl;
            xml_file << "    <description>" << (driver->description ? driver->description : "") << "</description>" << std::endl;
            xml_file << "    <param1_name>" << (driver->param1_name ? driver->param1_name : "") << "</param1_name>" << std::endl;
            xml_file << "    <param1_desc>" << (driver->param1_desc ? driver->param1_desc : "") << "</param1_desc>" << std::endl;
            xml_file << "    <param2_name>" << (driver->param2_name ? driver->param2_name : "") << "</param2_name>" << std::endl;
            xml_file << "    <param2_desc>" << (driver->param2_desc ? driver->param2_desc : "") << "</param2_desc>" << std::endl;
            xml_file << "    <param3_name>" << (driver->param3_name ? driver->param3_name : "") << "</param3_name>" << std::endl;
            xml_file << "    <param3_desc>" << (driver->param3_desc ? driver->param3_desc : "") << "</param3_desc>" << std::endl;
            xml_file << "    <param4_name>" << (driver->param4_name ? driver->param4_name : "") << "</param4_name>" << std::endl;
            xml_file << "    <param4_desc>" << (driver->param4_desc ? driver->param4_desc : "") << "</param4_desc>" << std::endl;
            
            if (driver->ppdevices && driver->device_count > 0) {
                xml_file << "    <devices>" << std::endl;
                for (int i = 0; i < driver->device_count; i++) {
                    auto device = driver->ppdevices[i];
                    if (device) {
                        xml_file << "      <device>" << std::endl;
                        xml_file << "        <name>" << (device->name ? device->name : "") << "</name>" << std::endl;
                        xml_file << "        <desc>" << (device->desc ? device->desc : "") << "</desc>" << std::endl;
                        xml_file << "        <conn_type>" << device->conn_type << "</conn_type>" << std::endl;
                        xml_file << "        <conn_param>" << (device->conn_param ? device->conn_param : "") << "</conn_param>" << std::endl;
                        xml_file << "        <param1>" << (device->param1 ? device->param1 : "") << "</param1>" << std::endl;
                        xml_file << "        <param2>" << (device->param2 ? device->param2 : "") << "</param2>" << std::endl;
                        xml_file << "        <param3>" << (device->param3 ? device->param3 : "") << "</param3>" << std::endl;
                        xml_file << "        <param4>" << (device->param4 ? device->param4 : "") << "</param4>" << std::endl;
                        
                        if (device->pptags && device->tag_count > 0) {
                            xml_file << "        <tags>" << std::endl;
                            for (int j = 0; j < device->tag_count; j++) {
                                auto tag = device->pptags[j];
                                if (tag) {
                                    xml_file << "          <tag>" << std::endl;
                                    xml_file << "            <name>" << (tag->name ? tag->name : "") << "</name>" << std::endl;
                                    xml_file << "            <address>" << (tag->address ? tag->address : "") << "</address>" << std::endl;
                                    xml_file << "            <data_type>" << tag->data_type << "</data_type>" << std::endl;
                                    xml_file << "            <point_type>" << tag->point_type << "</point_type>" << std::endl;
                                    xml_file << "            <transfer_type>" << tag->transfer_type << "</transfer_type>" << std::endl;
                                    xml_file << "            <polling_interval>" << tag->polling_interval << "</polling_interval>" << std::endl;
                                    xml_file << "            <linear_raw_min>" << tag->linear_raw_min << "</linear_raw_min>" << std::endl;
                                    xml_file << "            <linear_raw_max>" << tag->linear_raw_max << "</linear_raw_max>" << std::endl;
                                    xml_file << "            <linear_eng_min>" << tag->linear_eng_min << "</linear_eng_min>" << std::endl;
                                    xml_file << "            <linear_eng_max>" << tag->linear_eng_max << "</linear_eng_max>" << std::endl;
                                    xml_file << "            <advanced_algo_lib>" << (tag->advanced_algo_lib ? tag->advanced_algo_lib : "") << "</advanced_algo_lib>" << std::endl;
                                    xml_file << "            <advanced_param1>" << (tag->advanced_param1 ? tag->advanced_param1 : "") << "</advanced_param1>" << std::endl;
                                    xml_file << "            <advanced_param2>" << (tag->advanced_param2 ? tag->advanced_param2 : "") << "</advanced_param2>" << std::endl;
                                    xml_file << "            <advanced_param3>" << (tag->advanced_param3 ? tag->advanced_param3 : "") << "</advanced_param3>" << std::endl;
                                    xml_file << "            <advanced_param4>" << (tag->advanced_param4 ? tag->advanced_param4 : "") << "</advanced_param4>" << std::endl;
                                    xml_file << "            <enable_control>" << (tag->enable_control ? "true" : "false") << "</enable_control>" << std::endl;
                                    xml_file << "            <enable_history>" << (tag->enable_history ? "true" : "false") << "</enable_history>" << std::endl;
                                    xml_file << "          </tag>" << std::endl;
                                }
                            }
                            xml_file << "        </tags>" << std::endl;
                        }
                        
                        xml_file << "      </device>" << std::endl;
                    }
                }
                xml_file << "    </devices>" << std::endl;
            }
            
            xml_file << "  </driver>" << std::endl;
        }
        
        xml_file << "</drivers>" << std::endl;
        xml_file.close();
        
        return true;
    } catch (std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception when saving config to xml: %s", e.what());
        return false;
    }
}

bool ConfigLoader::LoadConfigFromDb()
{
    sqlite3* db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    if (rc != SQLITE_OK) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    try {
        // 清空现有驱动列表
        FreeDrivers();
        
        // 查询驱动列表
        const char* sql = "SELECT id, name, type, version, description, param1_name, param1_desc, param2_name, param2_desc, param3_name, param3_desc, param4_name, param4_desc FROM t_drivers";
        sqlite3_stmt* stmt = nullptr;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to prepare statement for drivers: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            int type = sqlite3_column_int(stmt, 2);
            const unsigned char* version = sqlite3_column_text(stmt, 3);
            const unsigned char* description = sqlite3_column_text(stmt, 4);
            const unsigned char* param1_name = sqlite3_column_text(stmt, 5);
            const unsigned char* param1_desc = sqlite3_column_text(stmt, 6);
            const unsigned char* param2_name = sqlite3_column_text(stmt, 7);
            const unsigned char* param2_desc = sqlite3_column_text(stmt, 8);
            const unsigned char* param3_name = sqlite3_column_text(stmt, 9);
            const unsigned char* param3_desc = sqlite3_column_text(stmt, 10);
            const unsigned char* param4_name = sqlite3_column_text(stmt, 11);
            const unsigned char* param4_desc = sqlite3_column_text(stmt, 12);
            
            // 创建驱动实例
            LWDRIVER* driver = CreateDriver(
                name ? reinterpret_cast<const char*>(name) : "",
                type,
                version ? reinterpret_cast<const char*>(version) : "",
                description ? reinterpret_cast<const char*>(description) : "",
                param1_name ? reinterpret_cast<const char*>(param1_name) : "",
                param1_desc ? reinterpret_cast<const char*>(param1_desc) : "",
                param2_name ? reinterpret_cast<const char*>(param2_name) : "",
                param2_desc ? reinterpret_cast<const char*>(param2_desc) : "",
                param3_name ? reinterpret_cast<const char*>(param3_name) : "",
                param3_desc ? reinterpret_cast<const char*>(param3_desc) : "",
                param4_name ? reinterpret_cast<const char*>(param4_name) : "",
                param4_desc ? reinterpret_cast<const char*>(param4_desc) : ""
            );
            
            if (driver) {
                drivers_.push_back(driver);
                
                // 查询该驱动下的设备列表
                const char* device_sql = "SELECT id, name, description, conn_type, connparam, param1, param2, param3, param4 FROM t_devices WHERE driver_id = ?";
                sqlite3_stmt* device_stmt = nullptr;
                rc = sqlite3_prepare_v2(db, device_sql, -1, &device_stmt, nullptr);
                if (rc == SQLITE_OK) {
                    sqlite3_bind_int(device_stmt, 1, id);
                    
                    std::vector<LWDEVICE*> devices;
                    while (sqlite3_step(device_stmt) == SQLITE_ROW) {
                        int device_id = sqlite3_column_int(device_stmt, 0);
                        const unsigned char* device_name = sqlite3_column_text(device_stmt, 1);
                        const unsigned char* device_desc = sqlite3_column_text(device_stmt, 2);
                        int device_conn_type = sqlite3_column_int(device_stmt, 3);
                        const unsigned char* device_connparam = sqlite3_column_text(device_stmt, 4);
                        const unsigned char* device_param1 = sqlite3_column_text(device_stmt, 5);
                        const unsigned char* device_param2 = sqlite3_column_text(device_stmt, 6);
                        const unsigned char* device_param3 = sqlite3_column_text(device_stmt, 7);
                        const unsigned char* device_param4 = sqlite3_column_text(device_stmt, 8);
                        
                        // 创建设备实例
                        LWDEVICE* device = CreateDevice(
                            device_name ? reinterpret_cast<const char*>(device_name) : "",
                            device_desc ? reinterpret_cast<const char*>(device_desc) : "",
                            device_conn_type,
                            device_connparam ? reinterpret_cast<const char*>(device_connparam) : "",
                            device_param1 ? reinterpret_cast<const char*>(device_param1) : "",
                            device_param2 ? reinterpret_cast<const char*>(device_param2) : "",
                            device_param3 ? reinterpret_cast<const char*>(device_param3) : "",
                            device_param4 ? reinterpret_cast<const char*>(device_param4) : ""
                        );
                        
                        if (device) {
                            device->driver = driver;
                            devices.push_back(device);
                            
                            // 查询该设备下的点位列表
                            const char* point_sql = "SELECT id, name, address, datatype, point_type, transfer_type, linear_raw_min, linear_raw_max, linear_eng_min, linear_eng_max, advanced_algo_lib, advanced_param1, advanced_param2, advanced_param3, advanced_param4, enable_control, enable_history, poll_rate FROM t_points WHERE device_id = ?";
                            sqlite3_stmt* point_stmt = nullptr;
                            rc = sqlite3_prepare_v2(db, point_sql, -1, &point_stmt, nullptr);
                            if (rc == SQLITE_OK) {
                                sqlite3_bind_int(point_stmt, 1, device_id);
                                
                                std::vector<LWTAG*> tags;
                                while (sqlite3_step(point_stmt) == SQLITE_ROW) {
                                    const unsigned char* point_name = sqlite3_column_text(point_stmt, 1);
                                    const unsigned char* point_address = sqlite3_column_text(point_stmt, 2);
                                    int point_datatype = sqlite3_column_int(point_stmt, 3);
                                    int point_type = sqlite3_column_int(point_stmt, 4);
                                    int transfer_type = sqlite3_column_int(point_stmt, 5);
                                    double linear_raw_min = sqlite3_column_double(point_stmt, 6);
                                    double linear_raw_max = sqlite3_column_double(point_stmt, 7);
                                    double linear_eng_min = sqlite3_column_double(point_stmt, 8);
                                    double linear_eng_max = sqlite3_column_double(point_stmt, 9);
                                    const unsigned char* advanced_algo_lib = sqlite3_column_text(point_stmt, 10);
                                    const unsigned char* advanced_param1 = sqlite3_column_text(point_stmt, 11);
                                    const unsigned char* advanced_param2 = sqlite3_column_text(point_stmt, 12);
                                    const unsigned char* advanced_param3 = sqlite3_column_text(point_stmt, 13);
                                    const unsigned char* advanced_param4 = sqlite3_column_text(point_stmt, 14);
                                    int enable_control = sqlite3_column_int(point_stmt, 15);
                                    int enable_history = sqlite3_column_int(point_stmt, 16);
                                    int poll_rate = sqlite3_column_int(point_stmt, 17);
                                    
                                    // 创建点位实例
                                    LWTAG* tag = CreateTag(
                                        point_name ? reinterpret_cast<const char*>(point_name) : "",
                                        point_address ? reinterpret_cast<const char*>(point_address) : "",
                                        point_datatype,
                                        point_type,
                                        transfer_type,
                                        poll_rate,
                                        linear_raw_min,
                                        linear_raw_max,
                                        linear_eng_min,
                                        linear_eng_max,
                                        advanced_algo_lib ? reinterpret_cast<const char*>(advanced_algo_lib) : "",
                                        advanced_param1 ? reinterpret_cast<const char*>(advanced_param1) : "",
                                        advanced_param2 ? reinterpret_cast<const char*>(advanced_param2) : "",
                                        advanced_param3 ? reinterpret_cast<const char*>(advanced_param3) : "",
                                        advanced_param4 ? reinterpret_cast<const char*>(advanced_param4) : "",
                                        enable_control != 0,
                                        enable_history != 0
                                    );
                                    
                                    if (tag) {
                                        tags.push_back(tag);
                                    }
                                }
                                
                                // 设置设备的点位列表
                                if (!tags.empty()) {
                                    device->pptags = new LWTAG*[tags.size()];
                                    device->tag_count = tags.size();
                                    for (size_t i = 0; i < tags.size(); i++) {
                                        device->pptags[i] = tags[i];
                                    }
                                }
                                
                                sqlite3_finalize(point_stmt);
                            } else {
                                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to prepare statement for points: %s", sqlite3_errmsg(db));
                            }
                        }
                    }
                    
                    // 设置驱动的设备列表
                    if (!devices.empty()) {
                        driver->ppdevices = new LWDEVICE*[devices.size()];
                        driver->device_count = devices.size();
                        for (size_t i = 0; i < devices.size(); i++) {
                            driver->ppdevices[i] = devices[i];
                        }
                    }
                    
                    sqlite3_finalize(device_stmt);
                } else {
                    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to prepare statement for devices: %s", sqlite3_errmsg(db));
                }
            }
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return true;
    } catch (std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception when loading config from database: %s", e.what());
        sqlite3_close(db);
        return false;
    }
}

bool ConfigLoader::LoadConfigFromXml()
{
    try {
        std::ifstream xml_file(config_path_);
        if (!xml_file.is_open()) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "Xml file not found: %s", config_path_.c_str());
            return false;
        }
        
        // 清空现有驱动列表
        FreeDrivers();
        
        // 这里简化处理，实际应该使用xml解析库
        // 例如：tinyxml2、rapidxml等
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Xml file found, loading config (xml parsing not implemented yet)");
        
        // TODO: 实现xml文件解析
        
        xml_file.close();
        return false; // 暂时返回false，等待实现xml解析
    } catch (std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception when loading config from xml: %s", e.what());
        return false;
    }
}

int ConfigLoader::ParseConnType(const std::string& conn_type_str)
{
    if (conn_type_str == "tcpclient") {
        return 1;
    } else if (conn_type_str == "tcpserver") {
        return 2;
    } else if (conn_type_str == "udp") {
        return 3;
    } else if (conn_type_str == "serial") {
        return 4;
    } else {
        return 0; // other
    }
}

int ConfigLoader::ParseDataType(const std::string& data_type_str)
{
    if (data_type_str == "bool") {
        return 1;
    } else if (data_type_str == "int") {
        return 2;
    } else if (data_type_str == "float") {
        return 3;
    } else if (data_type_str == "string") {
        return 4;
    } else {
        return 2; // default to int
    }
}

LWDRIVER* ConfigLoader::CreateDriver(const std::string& name, int type, const std::string& version, 
                                    const std::string& description, const std::string& param1_name, 
                                    const std::string& param1_desc, const std::string& param2_name, 
                                    const std::string& param2_desc, const std::string& param3_name, 
                                    const std::string& param3_desc, const std::string& param4_name, 
                                    const std::string& param4_desc)
{
    LWDRIVER* driver = new LWDRIVER();
    if (!driver) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver");
        return nullptr;
    }
    
    // 初始化驱动字段
    driver->name = strdup(name.c_str());
    driver->type = type;
    driver->version = strdup(version.c_str());
    driver->description = strdup(description.c_str());
    driver->param1_name = strdup(param1_name.c_str());
    driver->param1_desc = strdup(param1_desc.c_str());
    driver->param2_name = strdup(param2_name.c_str());
    driver->param2_desc = strdup(param2_desc.c_str());
    driver->param3_name = strdup(param3_name.c_str());
    driver->param3_desc = strdup(param3_desc.c_str());
    driver->param4_name = strdup(param4_name.c_str());
    driver->param4_desc = strdup(param4_desc.c_str());
    
    driver->ppdevices = nullptr;
    driver->device_count = 0;
    driver->_pInternalRef = nullptr;
    
    memset(driver->pUserData, 0, sizeof(driver->pUserData));
    memset(driver->nUserData, 0, sizeof(driver->nUserData));
    
    return driver;
}

LWDEVICE* ConfigLoader::CreateDevice(const std::string& name, const std::string& desc, int conn_type, 
                                    const std::string& conn_param, const std::string& param1, 
                                    const std::string& param2, const std::string& param3, 
                                    const std::string& param4)
{
    LWDEVICE* device = new LWDEVICE();
    if (!device) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for device");
        return nullptr;
    }
    
    // 初始化设备字段
    device->name = strdup(name.c_str());
    device->desc = strdup(desc.c_str());
    device->conn_type = conn_type;
    device->conn_param = strdup(conn_param.c_str());
    device->param1 = strdup(param1.c_str());
    device->param2 = strdup(param2.c_str());
    device->param3 = strdup(param3.c_str());
    device->param4 = strdup(param4.c_str());
    
    device->driver = nullptr;
    device->pptags = nullptr;
    device->tag_count = 0;
    device->_pInternalRef = nullptr;
    
    device->conn_timeout = 0;
    device->enable_connect = true;
    
    memset(device->pUserData, 0, sizeof(device->pUserData));
    memset(device->nUserData, 0, sizeof(device->nUserData));
    
    return device;
}

LWTAG* ConfigLoader::CreateTag(const std::string& name, const std::string& address, int data_type, 
                              int point_type, int transfer_type, int polling_interval, 
                              double linear_raw_min, double linear_raw_max, 
                              double linear_eng_min, double linear_eng_max, 
                              const std::string& advanced_algo_lib, const std::string& advanced_param1, 
                              const std::string& advanced_param2, const std::string& advanced_param3, 
                              const std::string& advanced_param4, bool enable_control, 
                              bool enable_history)
{
    LWTAG* tag = new LWTAG();
    if (!tag) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for tag");
        return nullptr;
    }
    
    // 初始化点位字段
    tag->name = strdup(name.c_str());
    tag->address = strdup(address.c_str());
    tag->data_type = data_type;
    tag->point_type = point_type;
    tag->transfer_type = transfer_type;
    tag->polling_interval = polling_interval;
    tag->linear_raw_min = linear_raw_min;
    tag->linear_raw_max = linear_raw_max;
    tag->linear_eng_min = linear_eng_min;
    tag->linear_eng_max = linear_eng_max;
    tag->advanced_algo_lib = strdup(advanced_algo_lib.c_str());
    tag->advanced_param1 = strdup(advanced_param1.c_str());
    tag->advanced_param2 = strdup(advanced_param2.c_str());
    tag->advanced_param3 = strdup(advanced_param3.c_str());
    tag->advanced_param4 = strdup(advanced_param4.c_str());
    tag->enable_control = enable_control;
    tag->enable_history = enable_history;
    
    tag->data = nullptr;
    tag->data_length = 0;
    tag->quantity = 0;
    tag->time_milli = 0;
    
    tag->res_data1 = 0;
    tag->res_data2 = 0;
    tag->res_data3 = 0;
    tag->res_data4 = 0;
    tag->res_pdata1 = nullptr;
    tag->res_pdata2 = nullptr;
    
    return tag;
}

void ConfigLoader::FreeDrivers()
{
    for (auto driver : drivers_) {
        if (driver) {
            // 释放设备列表
            if (driver->ppdevices && driver->device_count > 0) {
                for (int i = 0; i < driver->device_count; i++) {
                    LWDEVICE* device = driver->ppdevices[i];
                    if (device) {
                        // 释放点位列表
                        if (device->pptags && device->tag_count > 0) {
                            for (int j = 0; j < device->tag_count; j++) {
                                LWTAG* tag = device->pptags[j];
                                if (tag) {
                                    if (tag->name) free(tag->name);
                                    if (tag->address) free(tag->address);
                                    if (tag->advanced_algo_lib) free(tag->advanced_algo_lib);
                                    if (tag->advanced_param1) free(tag->advanced_param1);
                                    if (tag->advanced_param2) free(tag->advanced_param2);
                                    if (tag->advanced_param3) free(tag->advanced_param3);
                                    if (tag->advanced_param4) free(tag->advanced_param4);
                                    if (tag->data) free(tag->data);
                                    delete tag;
                                }
                            }
                            delete[] device->pptags;
                        }
                        
                        // 释放设备字段
                        if (device->name) free(device->name);
                        if (device->desc) free(device->desc);
                        if (device->conn_param) free(device->conn_param);
                        if (device->param1) free(device->param1);
                        if (device->param2) free(device->param2);
                        if (device->param3) free(device->param3);
                        if (device->param4) free(device->param4);
                        delete device;
                    }
                }
                delete[] driver->ppdevices;
            }
            
            // 释放驱动字段
            if (driver->name) free(driver->name);
            if (driver->version) free(driver->version);
            if (driver->description) free(driver->description);
            if (driver->param1_name) free(driver->param1_name);
            if (driver->param1_desc) free(driver->param1_desc);
            if (driver->param2_name) free(driver->param2_name);
            if (driver->param2_desc) free(driver->param2_desc);
            if (driver->param3_name) free(driver->param3_name);
            if (driver->param3_desc) free(driver->param3_desc);
            if (driver->param4_name) free(driver->param4_name);
            if (driver->param4_desc) free(driver->param4_desc);
            delete driver;
        }
    }
    
    drivers_.clear();
}
