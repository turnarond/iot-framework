#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include "plugin/StreamProtocolPlugin.h"

namespace video_server {

class PluginManager;

struct StreamInfo {
    std::string streamId;
    std::string deviceId;
    std::string protocol;
    std::string status;
    std::string sourceUrl;
    std::map<std::string, std::string> outputUrls;
    int clientCount = 0;
    long startTime = 0;
    long lastAccessTime = 0;
};

class StreamManager {
public:
    StreamManager();
    ~StreamManager();
    
    bool initialize(std::shared_ptr<PluginManager> pluginManager);
    bool uninitialize();
    
    bool startStream(const std::string& deviceId, const StreamConfig& config, std::string& streamId);
    bool stopStream(const std::string& streamId);
    
    StreamInfo getStreamInfo(const std::string& streamId);
    std::vector<StreamInfo> getAllStreams();
    
    std::string getStreamUrl(const std::string& streamId, const std::string& protocol);
    
    bool isStreamActive(const std::string& streamId);
    
    void incrementClientCount(const std::string& streamId);
    void decrementClientCount(const std::string& streamId);
    
    int getClientCount(const std::string& streamId);
    
private:
    std::string generateStreamId(const std::string& deviceId, const std::string& protocol);
    bool generateOutputUrls(StreamInfo& streamInfo, const StreamConfig& config);
    
    std::shared_ptr<PluginManager> pluginManager_;
    std::map<std::string, StreamInfo> streams_;
    std::mutex streamsMutex_;
};

} // namespace video_server

#endif // STREAM_MANAGER_H