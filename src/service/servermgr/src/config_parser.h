#pragma once

#include "servermgr.h"
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigParser {
public:
    static bool parse(const std::string& configPath, ServerConfig& config);
    static bool parseServiceFile(const std::string& configPath, ServiceConfig& config);
    
private:
    static bool parseServiceConfig(const json& j, ServiceConfig& config);
};
