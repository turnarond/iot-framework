#ifndef STREAM_PROTOCOL_PLUGIN_H
#define STREAM_PROTOCOL_PLUGIN_H

#include "PluginInterface.h"
#include <string>
#include <vector>

namespace video_server {

struct StreamConfig {
    std::string deviceId;
    std::string ip;
    int port;
    std::string username;
    std::string password;
    std::string path;
    std::string protocol;
    int timeout = 30;
    bool enableRTSP = true;
    bool enableRTMP = true;
    bool enableHLS = true;
    bool enableFLV = true;
    bool enableWebRTC = true;
};

class StreamProtocolPlugin : public PluginInterface {
public:
    virtual ~StreamProtocolPlugin() = default;
    
    virtual bool startStream(const std::string& streamId, const StreamConfig& config) = 0;
    virtual bool stopStream(const std::string& streamId) = 0;
    virtual std::string getStreamStatus(const std::string& streamId) = 0;
    virtual std::string generateStreamUrl(const std::string& streamId, const std::string& protocol) = 0;
    virtual bool isProtocolSupported(const std::string& protocol) = 0;
    virtual std::vector<std::string> getSupportedProtocols() const = 0;
};

} // namespace video_server

#endif // STREAM_PROTOCOL_PLUGIN_H