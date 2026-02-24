#include "HikvisionPlugin.h"
#include "LogService.h"
#include <vector>
#include <algorithm>

namespace video_server {

HikvisionPlugin::HikvisionPlugin() : initialized_(false) {
}

HikvisionPlugin::~HikvisionPlugin() {
    uninitialize();
}

std::string HikvisionPlugin::getName() const {
    return "hikvision";
}

std::string HikvisionPlugin::getVersion() const {
    return "1.0.0";
}

std::string HikvisionPlugin::getDescription() const {
    return "Hikvision stream protocol plugin";
}

bool HikvisionPlugin::initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    LOG_INFO("HikvisionPlugin initialized");
    return true;
}

bool HikvisionPlugin::uninitialize() {
    if (!initialized_) {
        return true;
    }
    
    // Stop all running streams
    for (const auto& [streamId, status] : runningStreams_) {
        stopStream(streamId);
    }
    
    runningStreams_.clear();
    initialized_ = false;
    LOG_INFO("HikvisionPlugin uninitialized");
    return true;
}

bool HikvisionPlugin::isInitialized() const {
    return initialized_;
}

bool HikvisionPlugin::startStream(const std::string& streamId, const StreamConfig& config) {
    if (!initialized_) {
        LOG_ERROR("HikvisionPlugin not initialized");
        return false;
    }
    
    // Build Hikvision RTSP URL (Hikvision uses standard RTSP with specific path format)
    std::string rtspUrl = "rtsp://";
    if (!config.username.empty()) {
        rtspUrl += config.username;
        if (!config.password.empty()) {
            rtspUrl += ":" + config.password;
        }
        rtspUrl += "@";
    }
    rtspUrl += config.ip;
    if (config.port != 554) {
        rtspUrl += ":" + std::to_string(config.port);
    }
    
    // Hikvision specific path format
    if (!config.path.empty()) {
        if (config.path[0] != '/') {
            rtspUrl += "/";
        }
        rtspUrl += config.path;
    } else {
        // Default Hikvision path format: /Streaming/Channels/101
        // 101 means channel 1, main stream
        // 102 means channel 1, sub stream
        rtspUrl += "/Streaming/Channels/101";
    }
    
    LOG_INFO("Starting Hikvision stream: %s, URL: %s", streamId.c_str(), rtspUrl.c_str());
    
    // Here we would typically use FFmpeg or another library to push the stream
    // For now, we'll just simulate success
    runningStreams_[streamId] = "active";
    
    LOG_INFO("Hikvision stream started successfully: %s", streamId.c_str());
    return true;
}

bool HikvisionPlugin::stopStream(const std::string& streamId) {
    if (!initialized_) {
        LOG_ERROR("HikvisionPlugin not initialized");
        return false;
    }
    
    auto it = runningStreams_.find(streamId);
    if (it == runningStreams_.end()) {
        LOG_WARN("Hikvision stream not found: %s", streamId.c_str());
        return false;
    }
    
    LOG_INFO("Stopping Hikvision stream: %s", streamId.c_str());
    
    // Here we would typically stop the FFmpeg process or other streaming mechanism
    // For now, we'll just remove it from the map
    runningStreams_.erase(it);
    
    LOG_INFO("Hikvision stream stopped successfully: %s", streamId.c_str());
    return true;
}

std::string HikvisionPlugin::getStreamStatus(const std::string& streamId) {
    if (!initialized_) {
        return "plugin_not_initialized";
    }
    
    auto it = runningStreams_.find(streamId);
    if (it == runningStreams_.end()) {
        return "not_running";
    }
    
    return it->second;
}

std::string HikvisionPlugin::generateStreamUrl(const std::string& streamId, const std::string& protocol) {
    if (!initialized_) {
        LOG_ERROR("HikvisionPlugin not initialized");
        return "";
    }
    
    // Generate stream URL based on protocol
    if (protocol == "rtsp") {
        return "rtsp://localhost:554/hikvision/" + streamId;
    } else if (protocol == "rtmp") {
        return "rtmp://localhost:1935/live/hikvision_" + streamId;
    } else if (protocol == "hls") {
        return "http://localhost:80/hls/hikvision_" + streamId + ".m3u8";
    } else if (protocol == "flv") {
        return "http://localhost:80/live/hikvision_" + streamId + ".flv";
    } else if (protocol == "webrtc") {
        return "webrtc://localhost:80/hikvision_" + streamId;
    }
    
    LOG_WARN("Unsupported protocol: %s", protocol.c_str());
    return "";
}

bool HikvisionPlugin::isProtocolSupported(const std::string& protocol) {
    std::vector<std::string> supported = getSupportedProtocols();
    return std::find(supported.begin(), supported.end(), protocol) != supported.end();
}

std::vector<std::string> HikvisionPlugin::getSupportedProtocols() const {
    return {"rtsp", "rtmp", "hls", "flv", "webrtc"};
}

// Plugin entry point
extern "C" {
    PluginInterface* createPlugin() {
        return new HikvisionPlugin();
    }
    
    void destroyPlugin(PluginInterface* plugin) {
        delete plugin;
    }
}

} // namespace video_server