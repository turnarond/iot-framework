#include "DatabaseManager.h"
#include "LogService.h"
#include <sqlite3.h>
#include <sstream>
#include <chrono>

namespace video_server {

DatabaseManager::DatabaseManager() 
    : dbConnection_(nullptr) {
}

DatabaseManager::~DatabaseManager() {
    uninitialize();
}

bool DatabaseManager::initialize() {
    // Get database file path
    std::string datapath = LWComm::GetDataPath();
    databaseFile_ = datapath + LW_OS_DIR_SEPARATOR + "bas-business.db";
    
    LOG_INFO("Initializing DatabaseManager with database file: %s", databaseFile_.c_str());
    
    // Initialize database connection
    if (!initDatabaseConnection()) {
        LOG_ERROR("Failed to initialize database connection");
        return false;
    }
    
    LOG_INFO("DatabaseManager initialized successfully");
    return true;
}

bool DatabaseManager::uninitialize() {
    closeDatabaseConnection();
    LOG_INFO("DatabaseManager uninitialized");
    return true;
}

bool DatabaseManager::initDatabaseConnection() {
    // Close existing connection if any
    closeDatabaseConnection();
    
    // Open database connection
    int result = sqlite3_open(databaseFile_.c_str(), (sqlite3**)&dbConnection_);
    if (result != SQLITE_OK) {
        LOG_ERROR("Failed to open database: %s", sqlite3_errmsg((sqlite3*)dbConnection_));
        dbConnection_ = nullptr;
        return false;
    }
    
    LOG_INFO("Database connection opened successfully");
    return true;
}

void DatabaseManager::closeDatabaseConnection() {
    if (dbConnection_) {
        sqlite3_close((sqlite3*)dbConnection_);
        dbConnection_ = nullptr;
        LOG_INFO("Database connection closed");
    }
}

bool DatabaseManager::executeQuery(const std::string& query) {
    if (!dbConnection_) {
        LOG_ERROR("Database connection not initialized");
        return false;
    }
    
    char* errorMessage = nullptr;
    int result = sqlite3_exec((sqlite3*)dbConnection_, query.c_str(), nullptr, nullptr, &errorMessage);
    
    if (result != SQLITE_OK) {
        LOG_ERROR("Failed to execute query: %s, error: %s", query.c_str(), errorMessage);
        sqlite3_free(errorMessage);
        return false;
    }
    
    return true;
}

std::vector<std::map<std::string, std::string>> DatabaseManager::executeSelectQuery(const std::string& query) {
    std::vector<std::map<std::string, std::string>> results;
    
    if (!dbConnection_) {
        LOG_ERROR("Database connection not initialized");
        return results;
    }
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2((sqlite3*)dbConnection_, query.c_str(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: %s, error: %s", query.c_str(), sqlite3_errmsg((sqlite3*)dbConnection_));
        return results;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::map<std::string, std::string> row;
        int columnCount = sqlite3_column_count(stmt);
        
        for (int i = 0; i < columnCount; i++) {
            const char* columnName = sqlite3_column_name(stmt, i);
            const char* columnValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            
            if (columnName && columnValue) {
                row[columnName] = columnValue;
            } else if (columnName) {
                row[columnName] = "";
            }
        }
        
        results.push_back(row);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

bool DatabaseManager::addCamera(const DeviceInfo& deviceInfo) {
    std::stringstream query;
    query << "INSERT INTO t_video_cameras "
          << "(name, model, ip, port, username, password, stream_protocol_id, stream_protocol_param, "
          << "control_protocol_id, control_protocol_param, partition_id, status, description, create_time) "
          << "VALUES ("
          << "'" << deviceInfo.name << "', "
          << "'" << deviceInfo.model << "', "
          << "'" << deviceInfo.ip << "', "
          << deviceInfo.port << ", "
          << "'" << deviceInfo.username << "', "
          << "'" << deviceInfo.password << "', "
          << "1, " // Default stream protocol ID (RTSP)
          << "'" << deviceInfo.streamProtocolParam << "', "
          << "1, " // Default control protocol ID (ONVIF)
          << "'" << deviceInfo.controlProtocolParam << "', "
          << (deviceInfo.partitionId.empty() ? "NULL" : "'" + deviceInfo.partitionId + "'") << ", "
          << "'offline', "
          << "'" << deviceInfo.description << "', "
          << deviceInfo.createTime << ")";
    
    return executeQuery(query.str());
}

bool DatabaseManager::removeCamera(const std::string& deviceId) {
    std::stringstream query;
    query << "DELETE FROM t_video_cameras WHERE id = " << deviceId;
    
    return executeQuery(query.str());
}

bool DatabaseManager::updateCamera(const std::string& deviceId, const DeviceInfo& deviceInfo) {
    std::stringstream query;
    query << "UPDATE t_video_cameras SET "
          << "name = '" << deviceInfo.name << "', "
          << "model = '" << deviceInfo.model << "', "
          << "ip = '" << deviceInfo.ip << "', "
          << "port = " << deviceInfo.port << ", "
          << "username = '" << deviceInfo.username << "', "
          << "password = '" << deviceInfo.password << "', "
          << "stream_protocol_param = '" << deviceInfo.streamProtocolParam << "', "
          << "control_protocol_param = '" << deviceInfo.controlProtocolParam << "', "
          << "partition_id = " << (deviceInfo.partitionId.empty() ? "NULL" : "'" + deviceInfo.partitionId + "'") << ", "
          << "description = '" << deviceInfo.description << "' "
          << "WHERE id = " << deviceId;
    
    return executeQuery(query.str());
}

bool DatabaseManager::updateCameraStatus(const std::string& deviceId, const std::string& status) {
    std::stringstream query;
    query << "UPDATE t_video_cameras SET status = '" << status << "' WHERE id = " << deviceId;
    
    return executeQuery(query.str());
}

std::vector<DeviceInfo> DatabaseManager::getAllCameras() {
    std::vector<DeviceInfo> cameras;
    
    std::string query = "SELECT * FROM t_video_cameras";
    auto results = executeSelectQuery(query);
    
    for (const auto& row : results) {
        DeviceInfo camera;
        auto it_id = row.find("id");
        if (it_id != row.end()) camera.deviceId = it_id->second;
        
        auto it_name = row.find("name");
        if (it_name != row.end()) camera.name = it_name->second;
        
        auto it_model = row.find("model");
        if (it_model != row.end()) camera.model = it_model->second;
        
        auto it_ip = row.find("ip");
        if (it_ip != row.end()) camera.ip = it_ip->second;
        
        auto it_port = row.find("port");
        if (it_port != row.end() && !it_port->second.empty()) {
            try {
                camera.port = std::stoi(it_port->second);
            } catch (...) {
                camera.port = 0;
            }
        }
        
        auto it_username = row.find("username");
        if (it_username != row.end()) camera.username = it_username->second;
        
        auto it_password = row.find("password");
        if (it_password != row.end()) camera.password = it_password->second;
        
        auto it_stream_param = row.find("stream_protocol_param");
        if (it_stream_param != row.end()) camera.streamProtocolParam = it_stream_param->second;
        
        auto it_control_param = row.find("control_protocol_param");
        if (it_control_param != row.end()) camera.controlProtocolParam = it_control_param->second;
        
        auto it_partition = row.find("partition_id");
        if (it_partition != row.end()) camera.partitionId = it_partition->second;
        
        auto it_status = row.find("status");
        if (it_status != row.end()) camera.status = it_status->second;
        
        auto it_desc = row.find("description");
        if (it_desc != row.end()) camera.description = it_desc->second;
        
        auto it_create_time = row.find("create_time");
        if (it_create_time != row.end() && !it_create_time->second.empty()) {
            try {
                camera.createTime = std::stol(it_create_time->second);
            } catch (...) {
                camera.createTime = 0;
            }
        }
        
        cameras.push_back(camera);
    }
    
    return cameras;
}

std::vector<DeviceInfo> DatabaseManager::getCamerasByPartition(const std::string& partitionId) {
    std::vector<DeviceInfo> cameras;
    
    std::stringstream query;
    query << "SELECT * FROM t_video_cameras WHERE partition_id = '" << partitionId << "'";
    auto results = executeSelectQuery(query.str());
    
    for (const auto& row : results) {
        DeviceInfo camera;
        auto it_id = row.find("id");
        if (it_id != row.end()) camera.deviceId = it_id->second;
        
        auto it_name = row.find("name");
        if (it_name != row.end()) camera.name = it_name->second;
        
        auto it_model = row.find("model");
        if (it_model != row.end()) camera.model = it_model->second;
        
        auto it_ip = row.find("ip");
        if (it_ip != row.end()) camera.ip = it_ip->second;
        
        auto it_port = row.find("port");
        if (it_port != row.end() && !it_port->second.empty()) {
            try {
                camera.port = std::stoi(it_port->second);
            } catch (...) {
                camera.port = 0;
            }
        }
        
        auto it_username = row.find("username");
        if (it_username != row.end()) camera.username = it_username->second;
        
        auto it_password = row.find("password");
        if (it_password != row.end()) camera.password = it_password->second;
        
        auto it_stream_param = row.find("stream_protocol_param");
        if (it_stream_param != row.end()) camera.streamProtocolParam = it_stream_param->second;
        
        auto it_control_param = row.find("control_protocol_param");
        if (it_control_param != row.end()) camera.controlProtocolParam = it_control_param->second;
        
        auto it_partition = row.find("partition_id");
        if (it_partition != row.end()) camera.partitionId = it_partition->second;
        
        auto it_status = row.find("status");
        if (it_status != row.end()) camera.status = it_status->second;
        
        auto it_desc = row.find("description");
        if (it_desc != row.end()) camera.description = it_desc->second;
        
        auto it_create_time = row.find("create_time");
        if (it_create_time != row.end() && !it_create_time->second.empty()) {
            try {
                camera.createTime = std::stol(it_create_time->second);
            } catch (...) {
                camera.createTime = 0;
            }
        }
        
        cameras.push_back(camera);
    }
    
    return cameras;
}

DeviceInfo DatabaseManager::getCameraById(const std::string& deviceId) {
    std::stringstream query;
    query << "SELECT * FROM t_video_cameras WHERE id = " << deviceId;
    auto results = executeSelectQuery(query.str());
    
    if (results.empty()) {
        DeviceInfo empty;
        empty.deviceId = deviceId;
        empty.status = "not_found";
        return empty;
    }
    
    const auto& row = results[0];
    DeviceInfo camera;
    auto it_id = row.find("id");
    if (it_id != row.end()) camera.deviceId = it_id->second;
    
    auto it_name = row.find("name");
    if (it_name != row.end()) camera.name = it_name->second;
    
    auto it_model = row.find("model");
    if (it_model != row.end()) camera.model = it_model->second;
    
    auto it_ip = row.find("ip");
    if (it_ip != row.end()) camera.ip = it_ip->second;
    
    auto it_port = row.find("port");
    if (it_port != row.end() && !it_port->second.empty()) {
        try {
            camera.port = std::stoi(it_port->second);
        } catch (...) {
            camera.port = 0;
        }
    }
    
    auto it_username = row.find("username");
    if (it_username != row.end()) camera.username = it_username->second;
    
    auto it_password = row.find("password");
    if (it_password != row.end()) camera.password = it_password->second;
    
    auto it_stream_param = row.find("stream_protocol_param");
    if (it_stream_param != row.end()) camera.streamProtocolParam = it_stream_param->second;
    
    auto it_control_param = row.find("control_protocol_param");
    if (it_control_param != row.end()) camera.controlProtocolParam = it_control_param->second;
    
    auto it_partition = row.find("partition_id");
    if (it_partition != row.end()) camera.partitionId = it_partition->second;
    
    auto it_status = row.find("status");
    if (it_status != row.end()) camera.status = it_status->second;
    
    auto it_desc = row.find("description");
    if (it_desc != row.end()) camera.description = it_desc->second;
    
    auto it_create_time = row.find("create_time");
    if (it_create_time != row.end() && !it_create_time->second.empty()) {
        try {
            camera.createTime = std::stol(it_create_time->second);
        } catch (...) {
            camera.createTime = 0;
        }
    }
    
    return camera;
}

bool DatabaseManager::addPartition(const std::string& name, const std::string& description, std::string& partitionId) {
    std::stringstream query;
    long createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    query << "INSERT INTO t_video_partitions (name, description, create_time) VALUES ("
          << "'" << name << "', "
          << "'" << description << "', "
          << createTime << ")";
    
    if (!executeQuery(query.str())) {
        return false;
    }
    
    // Get the last inserted ID
    sqlite3_stmt* stmt;
    std::string selectQuery = "SELECT last_insert_rowid()";
    int result = sqlite3_prepare_v2((sqlite3*)dbConnection_, selectQuery.c_str(), -1, &stmt, nullptr);
    
    if (result == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        partitionId = std::to_string(id);
        sqlite3_finalize(stmt);
        return true;
    }
    
    sqlite3_finalize(stmt);
    return false;
}

bool DatabaseManager::removePartition(const std::string& partitionId) {
    std::stringstream query;
    query << "DELETE FROM t_video_partitions WHERE id = " << partitionId;
    
    return executeQuery(query.str());
}

bool DatabaseManager::updatePartition(const std::string& partitionId, const std::string& name, const std::string& description) {
    std::stringstream query;
    query << "UPDATE t_video_partitions SET "
          << "name = '" << name << "', "
          << "description = '" << description << "' "
          << "WHERE id = " << partitionId;
    
    return executeQuery(query.str());
}

std::vector<std::pair<std::string, std::string>> DatabaseManager::getAllPartitions() {
    std::vector<std::pair<std::string, std::string>> partitions;
    
    std::string query = "SELECT id, name FROM t_video_partitions";
    auto results = executeSelectQuery(query);
    
    for (const auto& row : results) {
        std::string id, name;
        auto it_id = row.find("id");
        if (it_id != row.end()) id = it_id->second;
        
        auto it_name = row.find("name");
        if (it_name != row.end()) name = it_name->second;
        
        partitions.emplace_back(id, name);
    }
    
    return partitions;
}

std::vector<std::pair<std::string, std::string>> DatabaseManager::getAllStreamProtocols() {
    std::vector<std::pair<std::string, std::string>> protocols;
    
    std::string query = "SELECT id, name FROM t_dict_stream_protocols";
    auto results = executeSelectQuery(query);
    
    for (const auto& row : results) {
        std::string id, name;
        auto it_id = row.find("id");
        if (it_id != row.end()) id = it_id->second;
        
        auto it_name = row.find("name");
        if (it_name != row.end()) name = it_name->second;
        
        protocols.emplace_back(id, name);
    }
    
    return protocols;
}

std::string DatabaseManager::getStreamProtocolById(const std::string& protocolId) {
    std::stringstream query;
    query << "SELECT name FROM t_dict_stream_protocols WHERE id = " << protocolId;
    auto results = executeSelectQuery(query.str());
    
    if (results.empty()) {
        return "";
    }
    
    const auto& row = results[0];
    auto it_name = row.find("name");
    if (it_name != row.end()) {
        return it_name->second;
    }
    return "";
}

std::vector<std::pair<std::string, std::string>> DatabaseManager::getAllControlProtocols() {
    std::vector<std::pair<std::string, std::string>> protocols;
    
    std::string query = "SELECT id, name FROM t_dict_control_protocols";
    auto results = executeSelectQuery(query);
    
    for (const auto& row : results) {
        std::string id, name;
        auto it_id = row.find("id");
        if (it_id != row.end()) id = it_id->second;
        
        auto it_name = row.find("name");
        if (it_name != row.end()) name = it_name->second;
        
        protocols.emplace_back(id, name);
    }
    
    return protocols;
}

std::string DatabaseManager::getControlProtocolById(const std::string& protocolId) {
    std::stringstream query;
    query << "SELECT name FROM t_dict_control_protocols WHERE id = " << protocolId;
    auto results = executeSelectQuery(query.str());
    
    if (results.empty()) {
        return "";
    }
    
    const auto& row = results[0];
    auto it_name = row.find("name");
    if (it_name != row.end()) {
        return it_name->second;
    }
    return "";
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

} // namespace video_server