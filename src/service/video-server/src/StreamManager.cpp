#include "StreamManager.h"
#include "PluginManager.h"
#include "LogService.h"
#include <chrono>
#include <random>
#include <sstream>

namespace video_server {

StreamManager::StreamManager() {
}

StreamManager::~StreamManager() {
    uninitialize();
}

bool StreamManager::initialize(std::shared_ptr<PluginManager> pluginManager) {
    pluginManager_ = pluginManager;
    LOG_INFO("StreamManager initialized");
    return true;
}

bool StreamManager::uninitialize() {
    // Stop all streams
    for (const auto& [streamId, info] : streams_) {
        stopStream(streamId);
    }
    
    streams_.clear();
    LOG_INFO("StreamManager uninitialized");
    return true;
}

bool StreamManager::startStream(const std::string& deviceId, const StreamConfig& config, std::string& streamId) {
    // Generate stream ID
    streamId = generateStreamId(deviceId, config.protocol);
    
    // Check if stream already exists
    auto it = streams_.find(streamId);
    if (it != streams_.end()) {
        LOG_WARN("Stream already exists: %s", streamId.c_str());
        return true;
    }
    
    LOG_INFO("Starting stream: %s, device: %s, protocol: %s", streamId.c_str(), deviceId.c_str(), config.protocol.c_str());
    
    // Find appropriate stream protocol plugin
    auto plugin = pluginManager_->getStreamProtocolPluginByProtocol(config.protocol);
    if (!plugin) {
        // Try to get default RTSP plugin
        plugin = pluginManager_->getStreamProtocolPlugin("rtsp");
        if (!plugin) {
            LOG_ERROR("No suitable stream protocol plugin found for protocol: %s", config.protocol.c_str());
            return false;
        }
        LOG_WARN("No plugin found for protocol: %s, using RTSP plugin as fallback", config.protocol.c_str());
    }
    
    // Start stream using plugin
    if (!plugin->startStream(streamId, config)) {
        LOG_ERROR("Failed to start stream using plugin: %s", plugin->getName().c_str());
        return false;
    }
    
    // Create stream info
    StreamInfo info;
    info.streamId = streamId;
    info.deviceId = deviceId;
    info.protocol = config.protocol;
    info.status = "active";
    info.clientCount = 0;
    info.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    info.lastAccessTime = info.startTime;
    
    // Generate output URLs
    if (!generateOutputUrls(info, config)) {
        LOG_ERROR("Failed to generate output URLs");
        plugin->stopStream(streamId);
        return false;
    }
    
    // Store stream info
    {   std::lock_guard<std::mutex> lock(streamsMutex_);
        streams_[streamId] = info;
    }
    
    LOG_INFO("Stream started successfully: %s", streamId.c_str());
    return true;
}

bool StreamManager::stopStream(const std::string& streamId) {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it == streams_.end()) {
        LOG_WARN("Stream not found: %s", streamId.c_str());
        return false;
    }
    
    LOG_INFO("Stopping stream: %s", streamId.c_str());
    
    // Find appropriate stream protocol plugin
    auto plugin = pluginManager_->getStreamProtocolPluginByProtocol(it->second.protocol);
    if (!plugin) {
        // Try to get default RTSP plugin
        plugin = pluginManager_->getStreamProtocolPlugin("rtsp");
        if (!plugin) {
            LOG_ERROR("No suitable stream protocol plugin found");
            return false;
        }
    }
    
    // Stop stream using plugin
    if (!plugin->stopStream(streamId)) {
        LOG_ERROR("Failed to stop stream using plugin: %s", plugin->getName().c_str());
        return false;
    }
    
    // Remove stream info
    streams_.erase(it);
    
    LOG_INFO("Stream stopped successfully: %s", streamId.c_str());
    return true;
}

StreamInfo StreamManager::getStreamInfo(const std::string& streamId) {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it == streams_.end()) {
        StreamInfo empty;
        empty.streamId = streamId;
        empty.status = "not_found";
        return empty;
    }
    
    return it->second;
}

std::vector<StreamInfo> StreamManager::getAllStreams() {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    std::vector<StreamInfo> result;
    for (const auto& [streamId, info] : streams_) {
        result.push_back(info);
    }
    return result;
}

std::string StreamManager::getStreamUrl(const std::string& streamId, const std::string& protocol) {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it == streams_.end()) {
        LOG_WARN("Stream not found: %s", streamId.c_str());
        return "";
    }
    
    // Check if we already have this URL
    auto urlIt = it->second.outputUrls.find(protocol);
    if (urlIt != it->second.outputUrls.end()) {
        return urlIt->second;
    }
    
    // Generate URL using plugin
    auto plugin = pluginManager_->getStreamProtocolPluginByProtocol(it->second.protocol);
    if (!plugin) {
        // Try to get default RTSP plugin
        plugin = pluginManager_->getStreamProtocolPlugin("rtsp");
        if (!plugin) {
            LOG_ERROR("No suitable stream protocol plugin found");
            return "";
        }
    }
    
    std::string url = plugin->generateStreamUrl(streamId, protocol);
    if (!url.empty()) {
        // Store generated URL
        it->second.outputUrls[protocol] = url;
    }
    
    return url;
}

bool StreamManager::isStreamActive(const std::string& streamId) {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it == streams_.end()) {
        return false;
    }
    
    return it->second.status == "active";
}

void StreamManager::incrementClientCount(const std::string& streamId) {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it != streams_.end()) {
        it->second.clientCount++;
        it->second.lastAccessTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        LOG_DEBUG("Incremented client count for stream: %s, new count: %d", streamId.c_str(), it->second.clientCount);
    }
}

void StreamManager::decrementClientCount(const std::string& streamId) {
    std::unique_lock<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it != streams_.end()) {
        if (it->second.clientCount > 0) {
            it->second.clientCount--;
            LOG_DEBUG("Decremented client count for stream: %s, new count: %d", streamId.c_str(), it->second.clientCount);
            
            // Check if we should stop the stream due to no clients
            if (it->second.clientCount == 0) {
                LOG_INFO("No clients left for stream: %s, stopping stream", streamId.c_str());
                // We'll stop the stream outside the lock to avoid deadlocks
                std::string id = streamId;
                lock.unlock();
                stopStream(id);
            }
        }
    }
}

int StreamManager::getClientCount(const std::string& streamId) {
    std::lock_guard<std::mutex> lock(streamsMutex_);
    
    auto it = streams_.find(streamId);
    if (it == streams_.end()) {
        return 0;
    }
    
    return it->second.clientCount;
}

std::string StreamManager::generateStreamId(const std::string& deviceId, const std::string& protocol) {
    // Generate unique stream ID
    std::stringstream ss;
    ss << protocol << "_" << deviceId << "_";
    
    // Add timestamp
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    ss << timestamp << "_";
    
    // Add random suffix
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    ss << dis(gen);
    
    return ss.str();
}

bool StreamManager::generateOutputUrls(StreamInfo& streamInfo, const StreamConfig& config) {
    // Generate URLs for all enabled protocols
    std::vector<std::string> protocols = {"rtsp", "rtmp", "hls", "flv", "webrtc"};
    
    for (const auto& protocol : protocols) {
        bool enabled = false;
        if (protocol == "rtsp" && config.enableRTSP) enabled = true;
        if (protocol == "rtmp" && config.enableRTMP) enabled = true;
        if (protocol == "hls" && config.enableHLS) enabled = true;
        if (protocol == "flv" && config.enableFLV) enabled = true;
        if (protocol == "webrtc" && config.enableWebRTC) enabled = true;
        
        if (enabled) {
            std::string url = getStreamUrl(streamInfo.streamId, protocol);
            if (!url.empty()) {
                streamInfo.outputUrls[protocol] = url;
                LOG_DEBUG("Generated URL for stream %s, protocol %s: %s", streamInfo.streamId.c_str(), protocol.c_str(), url.c_str());
            }
        }
    }
    
    return true;
}

} // namespace video_server