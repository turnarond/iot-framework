#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "plugin/StreamProtocolPlugin.h"
#include "plugin/ControlProtocolPlugin.h"

namespace video_server {

class PluginManager {
public:
    PluginManager();
    ~PluginManager();
    
    bool initialize(const std::string& pluginDir);
    bool uninitialize();
    
    bool loadAllPlugins();
    bool unloadAllPlugins();
    bool loadPlugin(const std::string& pluginPath);
    
    std::vector<std::shared_ptr<StreamProtocolPlugin>> getStreamProtocolPlugins();
    std::vector<std::shared_ptr<ControlProtocolPlugin>> getControlProtocolPlugins();
    
    std::shared_ptr<StreamProtocolPlugin> getStreamProtocolPlugin(const std::string& name);
    std::shared_ptr<ControlProtocolPlugin> getControlProtocolPlugin(const std::string& name);
    
    std::shared_ptr<StreamProtocolPlugin> getStreamProtocolPluginByProtocol(const std::string& protocol);
    
    std::vector<std::string> getPluginNames() const;
    
private:
    
    std::string pluginDir_;
    std::vector<void*> pluginHandles_;
    std::vector<std::shared_ptr<PluginInterface>> plugins_;
    std::map<std::string, std::shared_ptr<StreamProtocolPlugin>> streamProtocolPlugins_;
    std::map<std::string, std::shared_ptr<ControlProtocolPlugin>> controlProtocolPlugins_;
    std::map<std::string, std::shared_ptr<StreamProtocolPlugin>> protocolToPlugin_;
};

} // namespace video_server

#endif // PLUGIN_MANAGER_H