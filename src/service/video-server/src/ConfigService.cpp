#include "ConfigService.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include "LogService.h"

namespace video_server {

ConfigService::ConfigService() {
}

ConfigService::~ConfigService() {
    uninitialize();
}

bool ConfigService::initialize(const std::string& configFile) {
    configFile_ = configFile;
    
    if (!loadConfig()) {
        LogService::instance().logError("Failed to load config file: %s", configFile_.c_str());
        return false;
    }
    
    LogService::instance().logInfo("ConfigService initialized with config file: %s", configFile_.c_str());
    return true;
}

bool ConfigService::uninitialize() {
    config_.clear();
    listeners_.clear();
    LogService::instance().logInfo("ConfigService uninitialized");
    return true;
}

bool ConfigService::loadConfig() {
    std::ifstream ifs(configFile_);
    if (!ifs.is_open()) {
        LogService::instance().logWarn("Config file not found, creating default config: %s", configFile_.c_str());
        
        // Create default config
        config_ = nlohmann::json({
            {"server", {
                {"host", "0.0.0.0"},
                {"port", 8080},
                {"rtsp_port", 554},
                {"rtmp_port", 1935},
                {"http_port", 80}
            }},
            {"zlmediakit", {
                {"enabled", true},
                {"path", "/usr/bin/MediaServer"},
                {"config_path", "/etc/zlmediakit/config.ini"}
            }},
            {"ffmpeg", {
                {"path", "/usr/bin/ffmpeg"}
            }},
            {"plugins", {
                {"directory", "./plugins"},
                {"enabled", {"rtsp", "hikvision", "dahua", "gb28181"}}
            }},
            {"logging", {
                {"level", "info"},
                {"directory", "./logs"}
            }},
            {"devices", {
                {"discovery_interval", 60},
                {"status_check_interval", 30}
            }}
        });
        
        return saveConfig();
    }
    
    try {
        ifs >> config_;
        LogService::instance().logInfo("Config file loaded successfully: %s", configFile_.c_str());
        return true;
    } catch (const nlohmann::json::exception& e) {
        LogService::instance().logError("Failed to parse config file: %s, error: %s", configFile_.c_str(), e.what());
        return false;
    }
}

bool ConfigService::saveConfig() {
    std::ofstream ofs(configFile_);
    if (!ofs.is_open()) {
        LogService::instance().logError("Failed to open config file for writing: %s", configFile_.c_str());
        return false;
    }
    
    try {
        ofs << std::setw(4) << config_ << std::endl;
        LogService::instance().logInfo("Config file saved successfully: %s", configFile_.c_str());
        return true;
    } catch (const nlohmann::json::exception& e) {
        LogService::instance().logError("Failed to save config file: %s, error: %s", configFile_.c_str(), e.what());
        return false;
    }
}

std::string ConfigService::get(const std::string& key, const std::string& defaultValue) {
    std::lock_guard<std::mutex> lock(configMutex_);
    
    try {
        nlohmann::json* current = &config_;
        size_t pos = 0;
        
        while (pos < key.size()) {
            size_t nextPos = key.find('.', pos);
            std::string segment;
            
            if (nextPos == std::string::npos) {
                segment = key.substr(pos);
                pos = key.size();
            } else {
                segment = key.substr(pos, nextPos - pos);
                pos = nextPos + 1;
            }
            
            if (current->find(segment) == current->end()) {
                return defaultValue;
            }
            
            if (pos >= key.size()) {
                if (current->at(segment).is_string()) {
                    return current->at(segment).get<std::string>();
                } else {
                    return current->at(segment).dump();
                }
            } else {
                current = &(*current)[segment];
            }
        }
        
        return defaultValue;
    } catch (const nlohmann::json::exception& e) {
        LogService::instance().logDebug("Failed to get config key: %s, error: %s", key.c_str(), e.what());
        return defaultValue;
    }
}

int ConfigService::getInt(const std::string& key, int defaultValue) {
    std::string value = get(key);
    if (value.empty()) {
        return defaultValue;
    }
    
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        LogService::instance().logDebug("Failed to convert config value to int: %s, error: %s", value.c_str(), e.what());
        return defaultValue;
    }
}

bool ConfigService::getBool(const std::string& key, bool defaultValue) {
    std::string value = get(key);
    if (value.empty()) {
        return defaultValue;
    }
    
    if (value == "true" || value == "1" || value == "yes") {
        return true;
    } else if (value == "false" || value == "0" || value == "no") {
        return false;
    }
    
    return defaultValue;
}

double ConfigService::getDouble(const std::string& key, double defaultValue) {
    std::string value = get(key);
    if (value.empty()) {
        return defaultValue;
    }
    
    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        LogService::instance().logDebug("Failed to convert config value to double: %s, error: %s", value.c_str(), e.what());
        return defaultValue;
    }
}

bool ConfigService::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(configMutex_);
    
    try {
        nlohmann::json* current = &config_;
        size_t pos = 0;
        
        while (pos < key.size()) {
            size_t nextPos = key.find('.', pos);
            std::string segment;
            
            if (nextPos == std::string::npos) {
                segment = key.substr(pos);
                pos = key.size();
            } else {
                segment = key.substr(pos, nextPos - pos);
                pos = nextPos + 1;
            }
            
            if (pos >= key.size()) {
                (*current)[segment] = value;
                break;
            } else {
                if (current->find(segment) == current->end()) {
                    (*current)[segment] = nlohmann::json::object();
                }
                current = &(*current)[segment];
            }
        }
        
        // Notify listeners
        auto it = listeners_.find(key);
        if (it != listeners_.end()) {
            for (auto& listener : it->second) {
                listener(key, value);
            }
        }
        
        LogService::instance().logDebug("Config set: %s = %s", key.c_str(), value.c_str());
        return true;
    } catch (const nlohmann::json::exception& e) {
        LogService::instance().logError("Failed to set config key: %s, error: %s", key.c_str(), e.what());
        return false;
    }
}

bool ConfigService::setInt(const std::string& key, int value) {
    return set(key, std::to_string(value));
}

bool ConfigService::setBool(const std::string& key, bool value) {
    return set(key, value ? "true" : "false");
}

bool ConfigService::setDouble(const std::string& key, double value) {
    return set(key, std::to_string(value));
}

bool ConfigService::addListener(const std::string& key, ConfigChangeListener listener) {
    std::lock_guard<std::mutex> lock(configMutex_);
    
    listeners_[key].push_back(listener);
    LogService::instance().logDebug("Config listener added for key: %s", key.c_str());
    return true;
}

bool ConfigService::removeListener(const std::string& key, ConfigChangeListener listener) {
    std::lock_guard<std::mutex> lock(configMutex_);
    
    auto it = listeners_.find(key);
    if (it != listeners_.end()) {
        auto& keyListeners = it->second;
        // 由于std::function没有默认的operator==，我们简单地清空所有监听器
        // 实际项目中可能需要使用更复杂的监听器管理方案
        keyListeners.clear();
        LogService::instance().logDebug("Config listeners cleared for key: %s", key.c_str());
        return true;
    }
    
    return false;
}

nlohmann::json ConfigService::getConfigJson() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return config_;
}

} // namespace video_server