#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include "DeviceManager.h"
#include "lwcomm/lwcomm.h"

namespace video_server {

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    bool initialize();
    bool uninitialize();
    
    // Camera operations
    bool addCamera(const DeviceInfo& deviceInfo);
    bool removeCamera(const std::string& deviceId);
    bool updateCamera(const std::string& deviceId, const DeviceInfo& deviceInfo);
    bool updateCameraStatus(const std::string& deviceId, const std::string& status);
    
    std::vector<DeviceInfo> getAllCameras();
    std::vector<DeviceInfo> getCamerasByPartition(const std::string& partitionId);
    DeviceInfo getCameraById(const std::string& deviceId);
    
    // Partition operations
    bool addPartition(const std::string& name, const std::string& description, std::string& partitionId);
    bool removePartition(const std::string& partitionId);
    bool updatePartition(const std::string& partitionId, const std::string& name, const std::string& description);
    std::vector<std::pair<std::string, std::string>> getAllPartitions();
    
    // Stream protocol operations
    std::vector<std::pair<std::string, std::string>> getAllStreamProtocols();
    std::string getStreamProtocolById(const std::string& protocolId);
    
    // Control protocol operations
    std::vector<std::pair<std::string, std::string>> getAllControlProtocols();
    std::string getControlProtocolById(const std::string& protocolId);
    
    static DatabaseManager& instance();
    
private:
    bool initDatabaseConnection();
    void closeDatabaseConnection();
    
    bool executeQuery(const std::string& query);
    std::vector<std::map<std::string, std::string>> executeSelectQuery(const std::string& query);
    
    std::string databaseFile_;
    void* dbConnection_;
};

} // namespace video_server

#endif // DATABASE_MANAGER_H