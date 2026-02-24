#ifndef ZL_MEDIA_KIT_ADAPTER_H
#define ZL_MEDIA_KIT_ADAPTER_H

#include <string>
#include <map>
#include <memory>

namespace video_server {

class ZLMediaKitAdapter {
public:
    ZLMediaKitAdapter();
    ~ZLMediaKitAdapter();
    
    bool initialize(const std::string& configPath = "");
    bool uninitialize();
    
    bool startServer();
    bool stopServer();
    
    bool isServerRunning() const;
    
    bool pushStream(const std::string& streamId, const std::string& sourceUrl);
    bool stopPushStream(const std::string& streamId);
    
    std::string generateStreamUrl(const std::string& streamId, const std::string& protocol);
    
    std::string getServerStatus();
    
    bool setConfig(const std::string& key, const std::string& value);
    std::string getConfig(const std::string& key);
    
private:
    bool checkServerStatus();
    std::string buildPushCommand(const std::string& streamId, const std::string& sourceUrl);
    std::string buildStreamUrl(const std::string& streamId, const std::string& protocol);
    
    bool serverRunning_;
    std::string configPath_;
    std::string mediaServerPath_;
    std::map<std::string, std::string> config_;
    std::map<std::string, std::string> pushingStreams_;
};

} // namespace video_server

#endif // ZL_MEDIA_KIT_ADAPTER_H