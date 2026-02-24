#include "VideoServer.h"
#include "LogService.h"
#include <filesystem>

namespace video_server {

namespace fs = std::filesystem;

VideoServer::VideoServer() 
    : running_(false) {
}

VideoServer::~VideoServer() {
    uninitialize();
}

bool VideoServer::initialize(const std::string& configFile) {
    configFile_ = configFile;
    
    // Initialize services
    if (!initializeServices()) {
        LOG_ERROR("Failed to initialize services");
        return false;
    }
    
    LOG_INFO("VideoServer initialized with config file: %s", configFile_.c_str());
    return true;
}

bool VideoServer::uninitialize() {
    if (running_) {
        stop();
    }
    
    // Uninitialize services
    uninitializeServices();
    
    LOG_INFO("VideoServer uninitialized");
    return true;
}

bool VideoServer::start() {
    if (running_) {
        LOG_INFO("VideoServer is already running");
        return true;
    }
    
    LOG_INFO("Starting VideoServer");
    
    // Start ZLMediaKit server
    if (zlMediaKitAdapter_) {
        if (!zlMediaKitAdapter_->startServer()) {
            LOG_ERROR("Failed to start ZLMediaKit server");
            return false;
        }
    }
    
    // Load cameras from database
    if (!loadCamerasFromDatabase()) {
        LOG_WARN("Failed to load cameras from database, continuing anyway");
    }
    
    running_ = true;
    LOG_INFO("VideoServer started successfully");
    return true;
}

bool VideoServer::stop() {
    if (!running_) {
        LOG_INFO("VideoServer is not running");
        return true;
    }
    
    LOG_INFO("Stopping VideoServer");
    
    // Stop ZLMediaKit server
    if (zlMediaKitAdapter_) {
        if (!zlMediaKitAdapter_->stopServer()) {
            LOG_WARN("Failed to stop ZLMediaKit server");
        }
    }
    
    running_ = false;
    LOG_INFO("VideoServer stopped");
    return true;
}

bool VideoServer::isRunning() const {
    return running_;
}

bool VideoServer::startStream(const std::string& deviceId, const StreamConfig& config, std::string& streamId) {
    if (!running_) {
        LOG_ERROR("VideoServer is not running");
        return false;
    }
    
    if (streamManager_) {
        return streamManager_->startStream(deviceId, config, streamId);
    }
    
    LOG_ERROR("StreamManager not initialized");
    return false;
}

bool VideoServer::stopStream(const std::string& streamId) {
    if (!running_) {
        LOG_ERROR("VideoServer is not running");
        return false;
    }
    
    if (streamManager_) {
        return streamManager_->stopStream(streamId);
    }
    
    LOG_ERROR("StreamManager not initialized");
    return false;
}

StreamInfo VideoServer::getStreamInfo(const std::string& streamId) {
    if (streamManager_) {
        return streamManager_->getStreamInfo(streamId);
    }
    
    StreamInfo empty;
    empty.streamId = streamId;
    empty.status = "server_not_running";
    return empty;
}

std::vector<StreamInfo> VideoServer::getAllStreams() {
    if (streamManager_) {
        return streamManager_->getAllStreams();
    }
    
    return {};
}

std::string VideoServer::getStreamUrl(const std::string& streamId, const std::string& protocol) {
    if (!running_) {
        LOG_ERROR("VideoServer is not running");
        return "";
    }
    
    if (streamManager_) {
        return streamManager_->getStreamUrl(streamId, protocol);
    }
    
    LOG_ERROR("StreamManager not initialized");
    return "";
}

bool VideoServer::addDevice(const DeviceInfo& deviceInfo) {
    if (!running_) {
        LOG_ERROR("VideoServer is not running");
        return false;
    }
    
    if (deviceManager_) {
        return deviceManager_->addDevice(deviceInfo);
    }
    
    LOG_ERROR("DeviceManager not initialized");
    return false;
}

bool VideoServer::removeDevice(const std::string& deviceId) {
    if (!running_) {
        LOG_ERROR("VideoServer is not running");
        return false;
    }
    
    if (deviceManager_) {
        return deviceManager_->removeDevice(deviceId);
    }
    
    LOG_ERROR("DeviceManager not initialized");
    return false;
}

bool VideoServer::updateDevice(const std::string& deviceId, const DeviceInfo& deviceInfo) {
    if (!running_) {
        LOG_ERROR("VideoServer is not running");
        return false;
    }
    
    if (deviceManager_) {
        return deviceManager_->updateDevice(deviceId, deviceInfo);
    }
    
    LOG_ERROR("DeviceManager not initialized");
    return false;
}

DeviceInfo VideoServer::getDeviceInfo(const std::string& deviceId) {
    if (deviceManager_) {
        return deviceManager_->getDeviceInfo(deviceId);
    }
    
    DeviceInfo empty;
    empty.deviceId = deviceId;
    empty.status = "server_not_running";
    return empty;
}

std::vector<DeviceInfo> VideoServer::getAllDevices() {
    if (deviceManager_) {
        return deviceManager_->getAllDevices();
    }
    
    return {};
}

bool VideoServer::loadPlugin(const std::string& pluginPath) {
    if (pluginManager_) {
        return pluginManager_->loadPlugin(pluginPath);
    }
    
    LOG_ERROR("PluginManager not initialized");
    return false;
}

bool VideoServer::unloadPlugin(const std::string& pluginName) {
    LOG_WARN("Unload plugin is not supported yet");
    return false;
}

std::vector<std::string> VideoServer::getPluginNames() {
    if (pluginManager_) {
        return pluginManager_->getPluginNames();
    }
    
    return {};
}

std::string VideoServer::getServerStatus() {
    std::stringstream status;
    
    status << "{";
    status << "\"running\": " << (running_ ? "true" : "false");
    
    // ZLMediaKit status
    if (zlMediaKitAdapter_) {
        status << ", \"zlmediakit\": \"" << zlMediaKitAdapter_->getServerStatus() << "\"";
    }
    
    // Stream count
    if (streamManager_) {
        auto streams = streamManager_->getAllStreams();
        status << ", \"stream_count\": " << streams.size();
    }
    
    // Device count
    if (deviceManager_) {
        auto devices = deviceManager_->getAllDevices();
        status << ", \"device_count\": " << devices.size();
    }
    
    // Plugin count
    if (pluginManager_) {
        auto plugins = pluginManager_->getPluginNames();
        status << ", \"plugin_count\": " << plugins.size();
    }
    
    status << "}";
    
    return status.str();
}

bool VideoServer::setConfig(const std::string& key, const std::string& value) {
    if (configService_) {
        return configService_->set(key, value);
    }
    
    LOG_ERROR("ConfigService not initialized");
    return false;
}

std::string VideoServer::getConfig(const std::string& key) {
    if (configService_) {
        return configService_->get(key);
    }
    
    LOG_ERROR("ConfigService not initialized");
    return "";
}

VideoServer& VideoServer::instance() {
    static VideoServer instance;
    return instance;
}

bool VideoServer::initializeServices() {
    // Initialize ConfigService
    configService_ = std::make_shared<ConfigService>();
    if (!configService_->initialize(configFile_)) {
        LOG_ERROR("Failed to initialize ConfigService");
        return false;
    }
    
    // Initialize LogService
    std::string logDir = configService_->get("logging.directory", "./logs");
    LogService::LogLevel logLevel = LogService::INFO;
    std::string levelStr = configService_->get("logging.level", "info");
    if (levelStr == "debug") {
        logLevel = LogService::DEBUG;
    } else if (levelStr == "warn") {
        logLevel = LogService::WARN;
    } else if (levelStr == "error") {
        logLevel = LogService::ERROR;
    } else if (levelStr == "critical") {
        logLevel = LogService::CRITICAL;
    }
    
    logService_ = std::make_shared<LogService>();
    if (!logService_->initialize(logDir, logLevel)) {
        LOG_ERROR("Failed to initialize LogService");
        return false;
    }
    
    // Initialize PluginManager
    std::string pluginDir = configService_->get("plugins.directory", "./plugins");
    pluginManager_ = std::make_shared<PluginManager>();
    if (!pluginManager_->initialize(pluginDir)) {
        LOG_ERROR("Failed to initialize PluginManager");
        return false;
    }
    
    // Load plugins
    if (!pluginManager_->loadAllPlugins()) {
        LOG_WARN("Failed to load all plugins");
    }
    
    // Initialize StreamManager
    streamManager_ = std::make_shared<StreamManager>();
    if (!streamManager_->initialize(pluginManager_)) {
        LOG_ERROR("Failed to initialize StreamManager");
        return false;
    }
    
    // Initialize DeviceManager
    deviceManager_ = std::make_shared<DeviceManager>();
    if (!deviceManager_->initialize(pluginManager_)) {
        LOG_ERROR("Failed to initialize DeviceManager");
        return false;
    }
    
    // Initialize DatabaseManager
    databaseManager_ = std::make_shared<DatabaseManager>();
    if (!databaseManager_->initialize()) {
        LOG_ERROR("Failed to initialize DatabaseManager");
        return false;
    }
    
    // Initialize ZLMediaKitAdapter
    std::string zlConfigPath = configService_->get("zlmediakit.config_path", "./config.ini");
    zlMediaKitAdapter_ = std::make_shared<ZLMediaKitAdapter>();
    if (!zlMediaKitAdapter_->initialize(zlConfigPath)) {
        LOG_ERROR("Failed to initialize ZLMediaKitAdapter");
        return false;
    }
    
    LOG_INFO("All services initialized successfully");
    return true;
}

bool VideoServer::loadCamerasFromDatabase() {
    if (!databaseManager_) {
        LOG_ERROR("DatabaseManager not initialized");
        return false;
    }
    
    LOG_INFO("Loading cameras from database");
    
    try {
        auto cameras = databaseManager_->getAllCameras();
        LOG_INFO("Found %d cameras in database", cameras.size());
        
        for (const auto& camera : cameras) {
            if (deviceManager_->addDevice(camera)) {
                LOG_INFO("Added camera from database: %s, IP: %s", camera.name.c_str(), camera.ip.c_str());
            } else {
                LOG_ERROR("Failed to add camera from database: %s", camera.name.c_str());
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception loading cameras from database: %s", e.what());
        return false;
    }
}

bool VideoServer::uninitializeServices() {
    // Uninitialize services in reverse order
    zlMediaKitAdapter_.reset();
    deviceManager_.reset();
    streamManager_.reset();
    databaseManager_.reset();
    pluginManager_.reset();
    logService_.reset();
    configService_.reset();
    
    LOG_INFO("All services uninitialized");
    return true;
}

} // namespace video_server