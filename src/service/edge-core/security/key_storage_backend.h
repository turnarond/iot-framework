
#pragma once
#include "security_key_manager.h"
#include "../cfgm/config_manager.h"
#include <string>

namespace edge {
namespace core {

// cfgm实现的存储后端
class ConfigManagerStorageBackend : public KeyStorageBackend {
public:
    explicit ConfigManagerStorageBackend(ConfigManager* configManager);
    ~ConfigManagerStorageBackend() = default;
    
    bool store(const std::string& key, const std::string& value) override;
    bool load(const std::string& key, std::string& value) const override;
    bool remove(const std::string& key) override;
    bool exists(const std::string& key) const override;

private:
    ConfigManager* configManager_;
};

} // namespace core
} // namespace edge