#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include "plugin/ControlProtocolPlugin.h"

namespace video_server {

class PluginManager;

struct DeviceInfo {
    std::string deviceId;
    std::string name;
    std::string model;
    std::string ip;
    int port;
    std::string username;
    std::string password;
    std::string streamProtocol;
    std::string streamProtocolParam;
    std::string controlProtocol;
    std::string controlProtocolParam;
    std::string partitionId;
    std::string status;
    std::string description;
    long createTime;
};

class DeviceManager {
public:
    DeviceManager();
    ~DeviceManager();
    
    bool initialize(std::shared_ptr<PluginManager> pluginManager);
    bool uninitialize();
    
    bool addDevice(const DeviceInfo& deviceInfo);
    bool removeDevice(const std::string& deviceId);
    bool updateDevice(const std::string& deviceId, const DeviceInfo& deviceInfo);
    
    DeviceInfo getDeviceInfo(const std::string& deviceId);
    std::vector<DeviceInfo> getAllDevices();
    std::vector<DeviceInfo> getDevicesByPartition(const std::string& partitionId);
    
    std::string getDeviceStatus(const std::string& deviceId);
    bool updateDeviceStatus(const std::string& deviceId, const std::string& status);
    
    bool connectDevice(const std::string& deviceId);
    bool disconnectDevice(const std::string& deviceId);
    
    bool sendDeviceCommand(const std::string& deviceId, const std::string& command, const nlohmann::json& params);
    
    bool discoverDevices(const std::string& subnet, std::vector<DeviceInfo>& discoveredDevices);
    
private:
    std::shared_ptr<PluginManager> pluginManager_;
    std::map<std::string, DeviceInfo> devices_;
    std::mutex devicesMutex_;
};

} // namespace video_server

#endif // DEVICE_MANAGER_H