#include "ZLMediaKitAdapter.h"
#include "LogService.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace video_server {

ZLMediaKitAdapter::ZLMediaKitAdapter() 
    : serverRunning_(false), 
      configPath_("./config.ini"), 
      mediaServerPath_("/usr/bin/MediaServer") {
    // Set default config
    config_["rtsp.port"] = "554";
    config_["rtmp.port"] = "1935";
    config_["http.port"] = "80";
    config_["rtc.port"] = "8443";
    config_["rtsp.enable"] = "1";
    config_["rtmp.enable"] = "1";
    config_["http.enable"] = "1";
    config_["rtc.enable"] = "1";
    config_["hls.enable"] = "1";
    config_["hls.segNum"] = "3";
    config_["hls.segDuration"] = "2";
    config_["flv.enable"] = "1";
}

ZLMediaKitAdapter::~ZLMediaKitAdapter() {
    uninitialize();
}

bool ZLMediaKitAdapter::initialize(const std::string& configPath) {
    if (!configPath.empty()) {
        configPath_ = configPath;
    }
    
    // Check if MediaServer exists
    struct stat statBuf;
    if (stat(mediaServerPath_.c_str(), &statBuf) != 0) {
        LOG_WARN("MediaServer not found at: %s, will use default path", mediaServerPath_.c_str());
        // Try to find MediaServer in common locations
        std::vector<std::string> paths = {
            "./MediaServer",
            "/usr/local/bin/MediaServer",
            "/opt/zlmediakit/bin/MediaServer"
        };
        
        bool found = false;
        for (const auto& path : paths) {
            if (stat(path.c_str(), &statBuf) == 0) {
                mediaServerPath_ = path;
                found = true;
                break;
            }
        }
        
        if (!found) {
            LOG_ERROR("MediaServer not found in any location");
            return false;
        }
    }
    
    LOG_INFO("ZLMediaKitAdapter initialized with MediaServer path: %s", mediaServerPath_.c_str());
    return true;
}

bool ZLMediaKitAdapter::uninitialize() {
    stopServer();
    LOG_INFO("ZLMediaKitAdapter uninitialized");
    return true;
}

bool ZLMediaKitAdapter::startServer() {
    if (serverRunning_) {
        LOG_INFO("ZLMediaKit server is already running");
        return true;
    }
    
    LOG_INFO("Starting ZLMediaKit server");
    
    // Create config file if it doesn't exist
    if (!configPath_.empty() && configPath_ != "/etc/zlmediakit/config.ini") {
        std::ofstream configFile(configPath_);
        if (configFile.is_open()) {
            for (const auto& [key, value] : config_) {
                configFile << key << "=" << value << std::endl;
            }
            configFile.close();
            LOG_INFO("Created ZLMediaKit config file: %s", configPath_.c_str());
        }
    }
    
    // Start MediaServer
    std::string command = mediaServerPath_;
    if (!configPath_.empty()) {
        command += " -c " + configPath_;
    }
    command += " > /dev/null 2>&1 &";
    
    LOG_INFO("Executing command: %s", command.c_str());
    
    int result = system(command.c_str());
    if (result != 0) {
        LOG_ERROR("Failed to start ZLMediaKit server, result: %d", result);
        return false;
    }
    
    // Wait a bit for server to start
    usleep(2000000); // 2 seconds
    
    if (checkServerStatus()) {
        serverRunning_ = true;
        LOG_INFO("ZLMediaKit server started successfully");
        return true;
    } else {
        LOG_ERROR("ZLMediaKit server started but not responding");
        return false;
    }
}

bool ZLMediaKitAdapter::stopServer() {
    if (!serverRunning_) {
        LOG_INFO("ZLMediaKit server is not running");
        return true;
    }
    
    LOG_INFO("Stopping ZLMediaKit server");
    
    // Stop MediaServer
    std::string command = "pkill -f MediaServer";
    int result = system(command.c_str());
    
    if (result != 0) {
        LOG_WARN("Failed to stop ZLMediaKit server, result: %d", result);
    }
    
    serverRunning_ = false;
    LOG_INFO("ZLMediaKit server stopped");
    return true;
}

bool ZLMediaKitAdapter::isServerRunning() const {
    return serverRunning_;
}

bool ZLMediaKitAdapter::pushStream(const std::string& streamId, const std::string& sourceUrl) {
    if (!serverRunning_) {
        LOG_ERROR("ZLMediaKit server is not running");
        return false;
    }
    
    LOG_INFO("Pushing stream to ZLMediaKit: %s, source URL: %s", streamId.c_str(), sourceUrl.c_str());
    
    // Build push command using FFmpeg
    std::string pushUrl = "rtsp://localhost:554/" + streamId;
    std::string ffmpegCommand = "ffmpeg -re -i " + sourceUrl + " -c:v copy -c:a copy -f rtsp " + pushUrl + " > /dev/null 2>&1 &";
    
    LOG_INFO("Executing FFmpeg command: %s", ffmpegCommand.c_str());
    
    int result = system(ffmpegCommand.c_str());
    if (result != 0) {
        LOG_ERROR("Failed to push stream, result: %d", result);
        return false;
    }
    
    // Store pushing stream
    pushingStreams_[streamId] = sourceUrl;
    
    LOG_INFO("Stream pushed successfully: %s", streamId.c_str());
    return true;
}

bool ZLMediaKitAdapter::stopPushStream(const std::string& streamId) {
    if (!serverRunning_) {
        LOG_ERROR("ZLMediaKit server is not running");
        return false;
    }
    
    LOG_INFO("Stopping push stream: %s", streamId.c_str());
    
    // Find and kill FFmpeg process
    std::string command = "pkill -f \"ffmpeg.*" + streamId + "\"";
    int result = system(command.c_str());
    
    if (result != 0) {
        LOG_WARN("Failed to stop push stream, result: %d", result);
    }
    
    // Remove from pushing streams
    pushingStreams_.erase(streamId);
    
    LOG_INFO("Push stream stopped: %s", streamId.c_str());
    return true;
}

std::string ZLMediaKitAdapter::generateStreamUrl(const std::string& streamId, const std::string& protocol) {
    return buildStreamUrl(streamId, protocol);
}

std::string ZLMediaKitAdapter::getServerStatus() {
    if (!serverRunning_) {
        return "stopped";
    }
    
    if (checkServerStatus()) {
        return "running";
    } else {
        return "error";
    }
}

bool ZLMediaKitAdapter::setConfig(const std::string& key, const std::string& value) {
    config_[key] = value;
    LOG_DEBUG("ZLMediaKit config set: %s = %s", key.c_str(), value.c_str());
    return true;
}

std::string ZLMediaKitAdapter::getConfig(const std::string& key) {
    auto it = config_.find(key);
    if (it != config_.end()) {
        return it->second;
    }
    return "";
}

bool ZLMediaKitAdapter::checkServerStatus() {
    // Check if MediaServer process is running
    std::string command = "pgrep -f MediaServer";
    std::string result = "";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
    }
    
    return !result.empty();
}

std::string ZLMediaKitAdapter::buildPushCommand(const std::string& streamId, const std::string& sourceUrl) {
    std::string pushUrl = "rtsp://localhost:554/" + streamId;
    return "ffmpeg -re -i " + sourceUrl + " -c:v copy -c:a copy -f rtsp " + pushUrl;
}

std::string ZLMediaKitAdapter::buildStreamUrl(const std::string& streamId, const std::string& protocol) {
    if (protocol == "rtsp") {
        return "rtsp://localhost:" + config_["rtsp.port"] + "/" + streamId;
    } else if (protocol == "rtmp") {
        return "rtmp://localhost:" + config_["rtmp.port"] + "/live/" + streamId;
    } else if (protocol == "hls") {
        return "http://localhost:" + config_["http.port"] + "/hls/" + streamId + ".m3u8";
    } else if (protocol == "flv") {
        return "http://localhost:" + config_["http.port"] + "/live/" + streamId + ".flv";
    } else if (protocol == "webrtc") {
        return "webrtc://localhost:" + config_["rtc.port"] + "/" + streamId;
    }
    
    LOG_WARN("Unsupported protocol: %s", protocol.c_str());
    return "";
}

} // namespace video_server