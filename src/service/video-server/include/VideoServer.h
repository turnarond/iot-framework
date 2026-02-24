#ifndef VIDEO_SERVER_H
#define VIDEO_SERVER_H

#include <string>
#include <memory>
#include "StreamManager.h"
#include "DeviceManager.h"
#include "PluginManager.h"
#include "ZLMediaKitAdapter.h"
#include "ConfigService.h"
#include "LogService.h"
#include "DatabaseManager.h"

namespace video_server {

class VideoServer {
public:
    VideoServer();
    ~VideoServer();
    
    bool initialize(const std::string& configFile);
    bool uninitialize();
    
    bool start();
    bool stop();
    
    bool isRunning() const;
    
    // Stream management
    bool startStream(const std::string& deviceId, const StreamConfig& config, std::string& streamId);
    bool stopStream(const std::string& streamId);
    StreamInfo getStreamInfo(const std::string& streamId);
    std::vector<StreamInfo> getAllStreams();
    std::string getStreamUrl(const std::string& streamId, const std::string& protocol);
    
    // Device management
    bool addDevice(const DeviceInfo& deviceInfo);
    bool removeDevice(const std::string& deviceId);
    bool updateDevice(const std::string& deviceId, const DeviceInfo& deviceInfo);
    DeviceInfo getDeviceInfo(const std::string& deviceId);
    std::vector<DeviceInfo> getAllDevices();
    
    // Plugin management
    bool loadPlugin(const std::string& pluginPath);
    bool unloadPlugin(const std::string& pluginName);
    std::vector<std::string> getPluginNames();
    
    // Server status
    std::string getServerStatus();
    
    // Config management
    bool setConfig(const std::string& key, const std::string& value);
    std::string getConfig(const std::string& key);
    
    static VideoServer& instance();
    
private:
    bool initializeServices();
    bool uninitializeServices();
    bool loadCamerasFromDatabase();
    
    bool running_;
    std::string configFile_;
    
    std::shared_ptr<ConfigService> configService_;
    std::shared_ptr<LogService> logService_;
    std::shared_ptr<PluginManager> pluginManager_;
    std::shared_ptr<StreamManager> streamManager_;
    std::shared_ptr<DeviceManager> deviceManager_;
    std::shared_ptr<ZLMediaKitAdapter> zlMediaKitAdapter_;
    std::shared_ptr<DatabaseManager> databaseManager_;
};

} // namespace video_server

#endif // VIDEO_SERVER_H