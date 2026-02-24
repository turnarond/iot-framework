#ifndef RTSP_PLUGIN_H
#define RTSP_PLUGIN_H

#include "plugin/StreamProtocolPlugin.h"
#include <map>

namespace video_server {

class RTSPPlugin : public StreamProtocolPlugin {
public:
    RTSPPlugin();
    ~RTSPPlugin() override;
    
    // PluginInterface
    std::string getName() const override;
    std::string getVersion() const override;
    std::string getDescription() const override;
    
    bool initialize() override;
    bool uninitialize() override;
    
    bool isInitialized() const override;
    
    // StreamProtocolPlugin
    bool startStream(const std::string& streamId, const StreamConfig& config) override;
    bool stopStream(const std::string& streamId) override;
    std::string getStreamStatus(const std::string& streamId) override;
    std::string generateStreamUrl(const std::string& streamId, const std::string& protocol) override;
    bool isProtocolSupported(const std::string& protocol) override;
    std::vector<std::string> getSupportedProtocols() const override;
    
private:
    bool initialized_;
    std::map<std::string, std::string> runningStreams_;
};

} // namespace video_server

#endif // RTSP_PLUGIN_H