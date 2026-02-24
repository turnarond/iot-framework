#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>

namespace video_server {

class ConfigService {
public:
    using ConfigChangeListener = std::function<void(const std::string& key, const std::string& value)>;
    
    ConfigService();
    ~ConfigService();
    
    bool initialize(const std::string& configFile);
    bool uninitialize();
    
    bool loadConfig();
    bool saveConfig();
    
    std::string get(const std::string& key, const std::string& defaultValue = "");
    int getInt(const std::string& key, int defaultValue = 0);
    bool getBool(const std::string& key, bool defaultValue = false);
    double getDouble(const std::string& key, double defaultValue = 0.0);
    
    bool set(const std::string& key, const std::string& value);
    bool setInt(const std::string& key, int value);
    bool setBool(const std::string& key, bool value);
    bool setDouble(const std::string& key, double value);
    
    bool addListener(const std::string& key, ConfigChangeListener listener);
    bool removeListener(const std::string& key, ConfigChangeListener listener);
    
    nlohmann::json getConfigJson() const;
    
private:
    std::string configFile_;
    nlohmann::json config_;
    mutable std::mutex configMutex_;
    std::map<std::string, std::vector<ConfigChangeListener>> listeners_;
};

} // namespace video_server

#endif // CONFIG_SERVICE_H