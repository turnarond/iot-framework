#include "RTSPPlugin.h"
#include "LogService.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

namespace video_server {

RTSPPlugin::RTSPPlugin() : initialized_(false) {
}

RTSPPlugin::~RTSPPlugin() {
    uninitialize();
}

std::string RTSPPlugin::getName() const {
    return "rtsp";
}

std::string RTSPPlugin::getVersion() const {
    return "1.0.0";
}

std::string RTSPPlugin::getDescription() const {
    return "RTSP stream protocol plugin";
}

bool RTSPPlugin::initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    LOG_INFO("RTSPPlugin initialized");
    return true;
}

bool RTSPPlugin::uninitialize() {
    if (!initialized_) {
        return true;
    }
    
    // Stop all running streams
    for (const auto& [streamId, status] : runningStreams_) {
        stopStream(streamId);
    }
    
    runningStreams_.clear();
    initialized_ = false;
    LOG_INFO("RTSPPlugin uninitialized");
    return true;
}

bool RTSPPlugin::isInitialized() const {
    return initialized_;
}

bool RTSPPlugin::startStream(const std::string& streamId, const StreamConfig& config) {
    if (!initialized_) {
        LOG_ERROR("RTSPPlugin not initialized");
        return false;
    }
    
    // Build RTSP URL
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
    if (!config.path.empty()) {
        if (config.path[0] != '/') {
            rtspUrl += "/";
        }
        rtspUrl += config.path;
    } else {
        rtspUrl += "/stream1";
    }
    
    LOG_INFO("Starting RTSP stream: %s, URL: %s", streamId.c_str(), rtspUrl.c_str());
    
    // Here we would typically use FFmpeg or another library to push the stream
    // For now, we'll just simulate success
    runningStreams_[streamId] = "active";
    
    LOG_INFO("RTSP stream started successfully: %s", streamId.c_str());
    return true;
}

bool RTSPPlugin::stopStream(const std::string& streamId) {
    if (!initialized_) {
        LOG_ERROR("RTSPPlugin not initialized");
        return false;
    }
    
    auto it = runningStreams_.find(streamId);
    if (it == runningStreams_.end()) {
        LOG_WARN("RTSP stream not found: %s", streamId.c_str());
        return false;
    }
    
    LOG_INFO("Stopping RTSP stream: %s", streamId.c_str());
    
    // Here we would typically stop the FFmpeg process or other streaming mechanism
    // For now, we'll just remove it from the map
    runningStreams_.erase(it);
    
    LOG_INFO("RTSP stream stopped successfully: %s", streamId.c_str());
    return true;
}

std::string RTSPPlugin::getStreamStatus(const std::string& streamId) {
    if (!initialized_) {
        return "plugin_not_initialized";
    }
    
    auto it = runningStreams_.find(streamId);
    if (it == runningStreams_.end()) {
        return "not_running";
    }
    
    return it->second;
}

std::string RTSPPlugin::generateStreamUrl(const std::string& streamId, const std::string& protocol) {
    if (!initialized_) {
        LOG_ERROR("RTSPPlugin not initialized");
        return "";
    }
    
    // Generate stream URL based on protocol
    if (protocol == "rtsp") {
        return "rtsp://localhost:554/" + streamId;
    } else if (protocol == "rtmp") {
        return "rtmp://localhost:1935/live/" + streamId;
    } else if (protocol == "hls") {
        return "http://localhost:80/hls/" + streamId + ".m3u8";
    } else if (protocol == "flv") {
        return "http://localhost:80/live/" + streamId + ".flv";
    } else if (protocol == "webrtc") {
        return "webrtc://localhost:80/" + streamId;
    }
    
    LOG_WARN("Unsupported protocol: %s", protocol.c_str());
    return "";
}

bool RTSPPlugin::isProtocolSupported(const std::string& protocol) {
    std::vector<std::string> supported = getSupportedProtocols();
    return std::find(supported.begin(), supported.end(), protocol) != supported.end();
}

std::vector<std::string> RTSPPlugin::getSupportedProtocols() const {
    return {"rtsp", "rtmp", "hls", "flv", "webrtc"};
}

// Plugin entry point
extern "C" {
    PluginInterface* createPlugin() {
        return new RTSPPlugin();
    }
    
    void destroyPlugin(PluginInterface* plugin) {
        delete plugin;
    }
}

} // namespace video_server