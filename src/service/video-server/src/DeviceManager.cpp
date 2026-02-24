#include "DeviceManager.h"
#include "PluginManager.h"
#include "LogService.h"
#include <chrono>

namespace video_server {

DeviceManager::DeviceManager() {
}

DeviceManager::~DeviceManager() {
    uninitialize();
}

bool DeviceManager::initialize(std::shared_ptr<PluginManager> pluginManager) {
    pluginManager_ = pluginManager;
    LOG_INFO("DeviceManager initialized");
    return true;
}

bool DeviceManager::uninitialize() {
    // Disconnect all devices
    for (const auto& [deviceId, info] : devices_) {
        disconnectDevice(deviceId);
    }
    
    devices_.clear();
    LOG_INFO("DeviceManager uninitialized");
    return true;
}

bool DeviceManager::addDevice(const DeviceInfo& deviceInfo) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    // Check if device already exists
    auto it = devices_.find(deviceInfo.deviceId);
    if (it != devices_.end()) {
        LOG_WARN("Device already exists: %s", deviceInfo.deviceId.c_str());
        return false;
    }
    
    LOG_INFO("Adding device: %s, IP: %s, port: %d", deviceInfo.name.c_str(), deviceInfo.ip.c_str(), deviceInfo.port);
    
    // Create device info
    DeviceInfo info = deviceInfo;
    if (info.createTime == 0) {
        info.createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    if (info.status.empty()) {
        info.status = "offline";
    }
    
    devices_[info.deviceId] = info;
    LOG_INFO("Device added successfully: %s", info.name.c_str());
    return true;
}

bool DeviceManager::removeDevice(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        LOG_WARN("Device not found: %s", deviceId.c_str());
        return false;
    }
    
    LOG_INFO("Removing device: %s", it->second.name.c_str());
    
    // Disconnect device
    disconnectDevice(deviceId);
    
    // Remove device
    devices_.erase(it);
    LOG_INFO("Device removed successfully: %s", deviceId.c_str());
    return true;
}

bool DeviceManager::updateDevice(const std::string& deviceId, const DeviceInfo& deviceInfo) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        LOG_WARN("Device not found: %s", deviceId.c_str());
        return false;
    }
    
    LOG_INFO("Updating device: %s", it->second.name.c_str());
    
    // Update device info
    DeviceInfo& info = it->second;
    info.name = deviceInfo.name;
    info.model = deviceInfo.model;
    info.ip = deviceInfo.ip;
    info.port = deviceInfo.port;
    info.username = deviceInfo.username;
    info.password = deviceInfo.password;
    info.streamProtocol = deviceInfo.streamProtocol;
    info.streamProtocolParam = deviceInfo.streamProtocolParam;
    info.controlProtocol = deviceInfo.controlProtocol;
    info.controlProtocolParam = deviceInfo.controlProtocolParam;
    info.partitionId = deviceInfo.partitionId;
    info.description = deviceInfo.description;
    
    LOG_INFO("Device updated successfully: %s", info.name.c_str());
    return true;
}

DeviceInfo DeviceManager::getDeviceInfo(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        DeviceInfo empty;
        empty.deviceId = deviceId;
        empty.status = "not_found";
        return empty;
    }
    
    return it->second;
}

std::vector<DeviceInfo> DeviceManager::getAllDevices() {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    std::vector<DeviceInfo> result;
    for (const auto& [deviceId, info] : devices_) {
        result.push_back(info);
    }
    return result;
}

std::vector<DeviceInfo> DeviceManager::getDevicesByPartition(const std::string& partitionId) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    std::vector<DeviceInfo> result;
    for (const auto& [deviceId, info] : devices_) {
        if (info.partitionId == partitionId) {
            result.push_back(info);
        }
    }
    return result;
}

std::string DeviceManager::getDeviceStatus(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        return "not_found";
    }
    
    return it->second.status;
}

bool DeviceManager::updateDeviceStatus(const std::string& deviceId, const std::string& status) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        LOG_WARN("Device not found: %s", deviceId.c_str());
        return false;
    }
    
    LOG_INFO("Updating device status: %s, old status: %s, new status: %s", 
             it->second.name.c_str(), it->second.status.c_str(), status.c_str());
    
    it->second.status = status;
    return true;
}

bool DeviceManager::connectDevice(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        LOG_WARN("Device not found: %s", deviceId.c_str());
        return false;
    }
    
    DeviceInfo& info = it->second;
    
    if (info.status == "online") {
        LOG_INFO("Device already connected: %s", info.name.c_str());
        return true;
    }
    
    LOG_INFO("Connecting device: %s, IP: %s", info.name.c_str(), info.ip.c_str());
    
    // Find appropriate control protocol plugin
    auto plugin = pluginManager_->getControlProtocolPlugin(info.controlProtocol);
    if (plugin) {
        // Connect using plugin
        DeviceConfig config;
        config.deviceId = deviceId;
        config.ip = info.ip;
        config.port = info.port;
        config.username = info.username;
        config.password = info.password;
        config.protocol = info.controlProtocol;
        
        if (plugin->connect(deviceId, config)) {
            info.status = "online";
            LOG_INFO("Device connected successfully: %s", info.name.c_str());
            return true;
        } else {
            LOG_ERROR("Failed to connect device using plugin: %s", plugin->getName().c_str());
            info.status = "error";
            return false;
        }
    } else {
        // Try to connect using default method (ping)
        LOG_WARN("No control protocol plugin found for device: %s, using ping to check status", info.name.c_str());
        
        // Ping device to check if it's reachable
        std::string command = "ping -c 1 -W 2 " + info.ip + " > /dev/null 2>&1";
        int result = system(command.c_str());
        
        if (result == 0) {
            info.status = "online";
            LOG_INFO("Device is reachable: %s", info.name.c_str());
            return true;
        } else {
            info.status = "offline";
            LOG_WARN("Device is not reachable: %s", info.name.c_str());
            return false;
        }
    }
}

bool DeviceManager::disconnectDevice(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        LOG_WARN("Device not found: %s", deviceId.c_str());
        return false;
    }
    
    DeviceInfo& info = it->second;
    
    if (info.status == "offline") {
        LOG_INFO("Device already disconnected: %s", info.name.c_str());
        return true;
    }
    
    LOG_INFO("Disconnecting device: %s", info.name.c_str());
    
    // Find appropriate control protocol plugin
    auto plugin = pluginManager_->getControlProtocolPlugin(info.controlProtocol);
    if (plugin) {
        // Disconnect using plugin
        if (plugin->disconnect(deviceId)) {
            info.status = "offline";
            LOG_INFO("Device disconnected successfully: %s", info.name.c_str());
            return true;
        } else {
            LOG_WARN("Failed to disconnect device using plugin: %s", plugin->getName().c_str());
            info.status = "offline";
            return true;
        }
    } else {
        // Just update status
        info.status = "offline";
        LOG_INFO("Device status updated to offline: %s", info.name.c_str());
        return true;
    }
}

bool DeviceManager::sendDeviceCommand(const std::string& deviceId, const std::string& command, const nlohmann::json& params) {
    std::lock_guard<std::mutex> lock(devicesMutex_);
    
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        LOG_WARN("Device not found: %s", deviceId.c_str());
        return false;
    }
    
    DeviceInfo& info = it->second;
    
    if (info.status != "online") {
        LOG_WARN("Device not online: %s", info.name.c_str());
        return false;
    }
    
    LOG_INFO("Sending command to device: %s, command: %s", info.name.c_str(), command.c_str());
    
    // Find appropriate control protocol plugin
    auto plugin = pluginManager_->getControlProtocolPlugin(info.controlProtocol);
    if (plugin) {
        // Send command using plugin
        if (plugin->sendCommand(deviceId, command, params)) {
            LOG_INFO("Command sent successfully to device: %s", info.name.c_str());
            return true;
        } else {
            LOG_ERROR("Failed to send command to device using plugin: %s", plugin->getName().c_str());
            return false;
        }
    } else {
        LOG_ERROR("No control protocol plugin found for device: %s", info.name.c_str());
        return false;
    }
}

bool DeviceManager::discoverDevices(const std::string& subnet, std::vector<DeviceInfo>& discoveredDevices) {
    LOG_INFO("Discovering devices in subnet: %s", subnet.c_str());
    
    // Here we would typically use SNMP, ONVIF discovery, or other methods to discover devices
    // For now, we'll just simulate discovery
    
    // Simulate discovering a few devices
    DeviceInfo device1;
    device1.deviceId = "discovered_1";
    device1.name = "Camera 1";
    device1.model = "IP Camera";
    device1.ip = subnet.substr(0, subnet.rfind(".") + 1) + "101";
    device1.port = 554;
    device1.streamProtocol = "RTSP";
    device1.controlProtocol = "ONVIF";
    device1.status = "offline";
    device1.createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    DeviceInfo device2;
    device2.deviceId = "discovered_2";
    device2.name = "Camera 2";
    device2.model = "IP Camera";
    device2.ip = subnet.substr(0, subnet.rfind(".") + 1) + "102";
    device2.port = 554;
    device2.streamProtocol = "RTSP";
    device2.controlProtocol = "ONVIF";
    device2.status = "offline";
    device2.createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    discoveredDevices.push_back(device1);
    discoveredDevices.push_back(device2);
    
    LOG_INFO("Discovered %d devices in subnet: %s", discoveredDevices.size(), subnet.c_str());
    return true;
}

} // namespace video_server