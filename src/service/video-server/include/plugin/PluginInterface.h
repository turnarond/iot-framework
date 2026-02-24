#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <string>
#include <vector>
#include <memory>

namespace video_server {

class PluginInterface {
public:
    virtual ~PluginInterface() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    
    virtual bool initialize() = 0;
    virtual bool uninitialize() = 0;
    
    virtual bool isInitialized() const = 0;
};

} // namespace video_server

#endif // PLUGIN_INTERFACE_H