#include "config_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ConfigParser::parse(const std::string& configPath, ServerConfig& config) 
{
    // Read config file
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << configPath << std::endl;
        return false;
    }
    
    try {
        // Parse JSON using nlohmann/json
        json j;
        file >> j;
        file.close();
        
        // Parse AppList
        if (j.contains("AppList")) {
            for (const auto& item : j["AppList"]) {
                ServiceConfig serviceConfig;
                parseServiceConfig(item, serviceConfig);
                config.appList.push_back(serviceConfig);
            }
        }
        
        // Parse OnStart
        if (j.contains("OnStart")) {
            for (const auto& item : j["OnStart"]) {
                ServiceConfig serviceConfig;
                parseServiceConfig(item, serviceConfig);
                config.onStart.push_back(serviceConfig);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON config: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

bool ConfigParser::parseServiceConfig(const json& j, ServiceConfig& config) 
{
    try {
        // Parse name
        if (j.contains("name")) {
            config.name = j["name"].get<std::string>();
        }
        
        // Parse executor
        if (j.contains("executor")) {
            config.executor = j["executor"].get<std::string>();
        }
        
        // Parse exePath
        if (j.contains("exePath")) {
            config.exePath = j["exePath"].get<std::string>();
        }
        
        // Parse args
        if (j.contains("args")) {
            for (const auto& arg : j["args"]) {
                config.args.push_back(arg.get<std::string>());
            }
        }
        
        // Parse delaySec
        if (j.contains("delaySec")) {
            config.delaySec = j["delaySec"].get<int>();
        }
        
        // Parse restart
        if (j.contains("restart")) {
            config.restart = j["restart"].get<bool>();
        }
        
        // Parse restartCount
        if (j.contains("restartCount")) {
            config.restartCount = j["restartCount"].get<int>();
        }
        
        // Parse stdout
        if (j.contains("stdout")) {
            config.stdout = j["stdout"].get<std::string>();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse service config: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

bool ConfigParser::parseServiceFile(const std::string& configPath, ServiceConfig& config) 
{
    // Read config file
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open service config file: " << configPath << std::endl;
        return false;
    }
    
    try {
        // Parse JSON using nlohmann/json
        json j;
        file >> j;
        file.close();
        
        // Parse service configuration
        return parseServiceConfig(j, config);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON service config: " << e.what() << std::endl;
        return false;
    }
}
