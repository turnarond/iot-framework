#include "PluginManager.h"
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include "LogService.h"

namespace video_server {

namespace fs = std::filesystem;

PluginManager::PluginManager() {
}

PluginManager::~PluginManager() {
    uninitialize();
}

bool PluginManager::initialize(const std::string& pluginDir) {
    pluginDir_ = pluginDir;
    
    // Create plugin directory if it doesn't exist
    if (!fs::exists(pluginDir_)) {
        try {
            fs::create_directories(pluginDir_);
            LOG_INFO("Created plugin directory: %s", pluginDir_.c_str());
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create plugin directory: %s, error: %s", pluginDir_.c_str(), e.what());
            return false;
        }
    }
    
    LOG_INFO("PluginManager initialized with plugin directory: %s", pluginDir_.c_str());
    return true;
}

bool PluginManager::uninitialize() {
    unloadAllPlugins();
    LOG_INFO("PluginManager uninitialized");
    return true;
}

bool PluginManager::loadAllPlugins() {
    try {
        for (const auto& entry : fs::directory_iterator(pluginDir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                if (!loadPlugin(entry.path().string())) {
                    LOG_WARN("Failed to load plugin: %s", entry.path().filename().c_str());
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to iterate plugin directory: %s, error: %s", pluginDir_.c_str(), e.what());
        return false;
    }
}

bool PluginManager::unloadAllPlugins() {
    for (auto& handle : pluginHandles_) {
        if (handle) {
            dlclose(handle);
        }
    }
    
    pluginHandles_.clear();
    plugins_.clear();
    streamProtocolPlugins_.clear();
    controlProtocolPlugins_.clear();
    protocolToPlugin_.clear();
    
    LOG_INFO("All plugins unloaded");
    return true;
}

bool PluginManager::loadPlugin(const std::string& pluginPath) {
    LOG_INFO("Loading plugin: %s", pluginPath.c_str());
    
    // Open the plugin
    void* handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
    if (!handle) {
        LOG_ERROR("Failed to open plugin: %s, error: %s", pluginPath.c_str(), dlerror());
        return false;
    }
    
    // Reset error
    dlerror();
    
    // Get plugin creation function
    typedef PluginInterface* (*CreatePluginFunc)();
    CreatePluginFunc createPlugin = (CreatePluginFunc)dlsym(handle, "createPlugin");
    
    if (!createPlugin) {
        LOG_ERROR("Failed to find createPlugin function in plugin: %s, error: %s", pluginPath.c_str(), dlerror());
        dlclose(handle);
        return false;
    }
    
    // Create plugin instance
    PluginInterface* plugin = createPlugin();
    if (!plugin) {
        LOG_ERROR("Failed to create plugin instance from: %s", pluginPath.c_str());
        dlclose(handle);
        return false;
    }
    
    // Initialize plugin
    if (!plugin->initialize()) {
        LOG_ERROR("Failed to initialize plugin: %s", plugin->getName().c_str());
        delete plugin;
        dlclose(handle);
        return false;
    }
    
    // Store plugin
    pluginHandles_.push_back(handle);
    plugins_.push_back(std::shared_ptr<PluginInterface>(plugin));
    
    // Check plugin type
    if (auto streamPlugin = dynamic_cast<StreamProtocolPlugin*>(plugin)) {
        streamProtocolPlugins_[plugin->getName()] = std::shared_ptr<StreamProtocolPlugin>(streamPlugin);
        
        // Map supported protocols to plugin
        for (const auto& protocol : streamPlugin->getSupportedProtocols()) {
            protocolToPlugin_[protocol] = std::shared_ptr<StreamProtocolPlugin>(streamPlugin);
            LOG_DEBUG("Mapped protocol %s to plugin %s", protocol.c_str(), plugin->getName().c_str());
        }
        
        LOG_INFO("Loaded stream protocol plugin: %s (version: %s)", plugin->getName().c_str(), plugin->getVersion().c_str());
    } else if (auto controlPlugin = dynamic_cast<ControlProtocolPlugin*>(plugin)) {
        controlProtocolPlugins_[plugin->getName()] = std::shared_ptr<ControlProtocolPlugin>(controlPlugin);
        LOG_INFO("Loaded control protocol plugin: %s (version: %s)", plugin->getName().c_str(), plugin->getVersion().c_str());
    } else {
        LOG_WARN("Loaded unknown plugin type: %s", plugin->getName().c_str());
    }
    
    return true;
}

std::vector<std::shared_ptr<StreamProtocolPlugin>> PluginManager::getStreamProtocolPlugins() {
    std::vector<std::shared_ptr<StreamProtocolPlugin>> result;
    for (const auto& [name, plugin] : streamProtocolPlugins_) {
        result.push_back(plugin);
    }
    return result;
}

std::vector<std::shared_ptr<ControlProtocolPlugin>> PluginManager::getControlProtocolPlugins() {
    std::vector<std::shared_ptr<ControlProtocolPlugin>> result;
    for (const auto& [name, plugin] : controlProtocolPlugins_) {
        result.push_back(plugin);
    }
    return result;
}

std::shared_ptr<StreamProtocolPlugin> PluginManager::getStreamProtocolPlugin(const std::string& name) {
    auto it = streamProtocolPlugins_.find(name);
    if (it != streamProtocolPlugins_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<ControlProtocolPlugin> PluginManager::getControlProtocolPlugin(const std::string& name) {
    auto it = controlProtocolPlugins_.find(name);
    if (it != controlProtocolPlugins_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<StreamProtocolPlugin> PluginManager::getStreamProtocolPluginByProtocol(const std::string& protocol) {
    auto it = protocolToPlugin_.find(protocol);
    if (it != protocolToPlugin_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::getPluginNames() const {
    std::vector<std::string> names;
    for (const auto& plugin : plugins_) {
        names.push_back(plugin->getName());
    }
    return names;
}

} // namespace video_server