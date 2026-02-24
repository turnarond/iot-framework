#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>

struct ServiceConfig {
    std::string name;
    std::string executor;
    std::string exePath;
    std::vector<std::string> args;
    int delaySec;
    bool restart;
    int restartCount;
    std::string stdout;
};

struct ServerConfig {
    std::vector<ServiceConfig> appList;
    std::vector<ServiceConfig> onStart;
};

class ServiceInstance {
public:
    ServiceInstance(const ServiceConfig& config, const std::string& serverPath = "");
    ~ServiceInstance();
    
    bool start();
    bool stop();
    bool restart();
    bool isRunning();
    
    std::string getName() const { return config.name; }
    pid_t getPid() const { return pid; }
    std::string getStartTime() const { return startTime; }
    
private:
    ServiceConfig config;
    std::string serverPath; // Path to servermgr executable
    pid_t pid;
    bool running;
    std::mutex mutex;
    int restartAttempts;
    std::string startTime; // Start time of the service
    
    bool execute();
    void monitor();
};

class ServerManager {
public:
    ServerManager(const std::string& executablePath = "");
    ~ServerManager();
    
    bool loadConfig(const std::string& configPath);
    bool loadConfigDirectory(const std::string& configDir);
    bool reloadConfig();
    bool startService(const std::string& serviceName);
    bool stopService(const std::string& serviceName);
    bool restartService(const std::string& serviceName);
    std::vector<std::string> listServices();
    std::vector<std::tuple<std::string, bool, std::string>> listServicesWithStatusAndTime();
    bool startAllServices();
    
    std::string getExecutablePath() const { return executablePath; }
    
private:
    ServerConfig config;
    std::vector<std::shared_ptr<ServiceInstance>> services;
    std::mutex mutex;
    std::string executablePath; // Path to servermgr executable
    
    void startOnBootServices();
};
