#ifndef CONTROL_PROTOCOL_PLUGIN_H
#define CONTROL_PROTOCOL_PLUGIN_H

#include "PluginInterface.h"
#include <string>
#include <nlohmann/json.hpp>

namespace video_server {

struct DeviceConfig {
    std::string deviceId;
    std::string ip;
    int port;
    std::string username;
    std::string password;
    std::string protocol;
    int timeout = 30;
};

class ControlProtocolPlugin : public PluginInterface {
public:
    virtual ~ControlProtocolPlugin() = default;
    
    virtual bool connect(const std::string& deviceId, const DeviceConfig& config) = 0;
    virtual bool disconnect(const std::string& deviceId) = 0;
    virtual bool sendCommand(const std::string& deviceId, const std::string& command, const nlohmann::json& params) = 0;
    virtual nlohmann::json getDeviceInfo(const std::string& deviceId) = 0;
    virtual bool isProtocolSupported(const std::string& protocol) = 0;
    virtual std::vector<std::string> getSupportedCommands() const = 0;
};

} // namespace video_server

#endif // CONTROL_PROTOCOL_PLUGIN_H