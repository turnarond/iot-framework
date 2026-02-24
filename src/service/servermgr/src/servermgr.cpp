#include "servermgr.h"
#include "config_parser.h"
#include "lwcomm/lwcomm.h"
#include "lwlog/lwlog.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <spawn.h>
#include <sstream>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstdlib>

extern char **environ;
extern CLWLog g_logger;
ServiceInstance::ServiceInstance(const ServiceConfig& config, const std::string& serverPath)
    : config(config), serverPath(serverPath), pid(-1), running(false), restartAttempts(0), startTime("") 
{
}

ServiceInstance::~ServiceInstance() 
{
    stop();
}

bool ServiceInstance::start() 
{
    std::lock_guard<std::mutex> lock(mutex);
    
    if (running) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Service %s is already running", 
            config.name.c_str());
        return true;
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Starting service: %s", 
        config.name.c_str());
    
    if (config.delaySec > 0) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Delaying start of service %s by %d seconds", 
            config.name.c_str(), config.delaySec);
        std::this_thread::sleep_for(std::chrono::seconds(config.delaySec));
    }
    
    // Print service configuration for debugging
    std::stringstream cmdStream;
    if (!config.executor.empty()) {
        cmdStream << config.executor << " ";
    }
    cmdStream << config.exePath;
    for (const auto& arg : config.args) {
        cmdStream << " " << arg;
    }
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Executing command for service %s: %s", 
        config.name.c_str(), cmdStream.str().c_str());
    
    bool success = execute();
    if (success) {
        running = true;
        restartAttempts = 0;
        
        // Set start time
        time_t now = time(nullptr);
        struct tm* tm_info = localtime(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        startTime = buffer;
        
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Service %s started successfully with PID: %d at %s", 
            config.name.c_str(), pid, startTime.c_str());
        std::thread monitorThread(&ServiceInstance::monitor, this);
        monitorThread.detach();
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start service: %s", config.name.c_str());
    }
    
    return success;
}

bool ServiceInstance::stop() 
{
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!running) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Service %s is not running", 
            config.name.c_str());
        return true;
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Stopping service: %s", 
        config.name.c_str());
    
    if (pid > 0) {
        kill(pid, SIGTERM);
        // Wait for process to exit
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Service %s exited with status %d", 
                config.name.c_str(), WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Service %s terminated by signal %d", 
                config.name.c_str(), WTERMSIG(status));
        }
    }
    
    running = false;
    pid = -1;
    return true;
}

bool ServiceInstance::restart() 
{
    stop();
    return start();
}

bool ServiceInstance::isRunning() 
{
    std::lock_guard<std::mutex> lock(mutex);
    return running;
}

bool ServiceInstance::execute() 
{
    // Prepare command arguments
    std::vector<const char*> cmdArgs;
    
    std::string executablePath;
    
    // Check if exePath is a relative path
    if (config.exePath.empty()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Executable path is empty for service %s", 
            config.name.c_str());
        return false;
    }
    
    if (config.exePath[0] != '/') {
        // Relative path, use servermgr's directory
        if (!serverPath.empty()) {
            // Extract directory from serverPath
            size_t lastSlash = serverPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                std::string serverDir = serverPath.substr(0, lastSlash);
                executablePath = serverDir + "/" + config.exePath;
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Using relative path for service %s: %s -> %s", 
                    config.name.c_str(), config.exePath.c_str(), executablePath.c_str());
            } else {
                // No directory in serverPath, use current directory
                executablePath = config.exePath;
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Using current directory for service %s: %s", 
                    config.name.c_str(), config.exePath.c_str());
            }
        } else {
            // Server path not set, use current directory
            executablePath = config.exePath;
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Server path not set, using current directory for service %s: %s", 
                config.name.c_str(), config.exePath.c_str());
        }
    } else {
        // Absolute path, use as is
        executablePath = config.exePath;
    }
    
    if (!config.executor.empty()) {
        cmdArgs.push_back(config.executor.c_str());
        cmdArgs.push_back(executablePath.c_str());
    } else {
        cmdArgs.push_back(executablePath.c_str());
    }
    
    for (const auto& arg : config.args) {
        cmdArgs.push_back(arg.c_str());
    }
    
    cmdArgs.push_back(nullptr);
    
    // Check if executable exists
    if (access(cmdArgs[0], F_OK) != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Executable not found for service %s: %s", 
            config.name.c_str(), cmdArgs[0]);
        return false;
    }
    
    if (access(cmdArgs[0], X_OK) != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Executable not executable for service %s: %s", 
            config.name.c_str(), cmdArgs[0]);
        return false;
    }
    
    // Prepare file actions for stdout/stderr redirection
    posix_spawn_file_actions_t fileActions;
    if (posix_spawn_file_actions_init(&fileActions) != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize file actions for service: %s", 
            config.name.c_str());
        return false;
    }
    
    // Redirect stdout/stderr if disabled
    if (config.stdout == "disabled") {
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull != -1) {
            posix_spawn_file_actions_adddup2(&fileActions, devNull, STDOUT_FILENO);
            posix_spawn_file_actions_adddup2(&fileActions, devNull, STDERR_FILENO);
            posix_spawn_file_actions_addclose(&fileActions, devNull);
        }
    }
    
    // Spawn the process with environment variables
    int result = posix_spawn(&pid, cmdArgs[0], &fileActions, nullptr, 
                             const_cast<char* const*>(cmdArgs.data()), environ);
    
    // Cleanup file actions
    posix_spawn_file_actions_destroy(&fileActions);
    
    if (result != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to spawn process for service: %s, error: %d, %s", 
                          config.name.c_str(), result, strerror(result));
        return false;
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Successfully spawned process for service %s with PID: %d", 
        config.name.c_str(), pid);
    
    return true;
}

void ServiceInstance::monitor() 
{
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!running) break;
        }
        
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == pid) {
            // Process exited
            std::lock_guard<std::mutex> lock(mutex);
            running = false;
            pid = -1;
            
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Service %s exited", config.name.c_str());
            
            // Handle restart
            if (config.restart && restartAttempts < config.restartCount) {
                restartAttempts++;
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Restarting service %s (attempt %d/%d)", 
                    config.name.c_str(), restartAttempts, config.restartCount);
                running = execute();
                if (running) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
            }
            
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

ServerManager::ServerManager(const std::string& executablePath) : executablePath(executablePath) {
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "ServerManager initialized with executable path: %s", 
        executablePath.empty() ? "not set" : executablePath.c_str());
}

ServerManager::~ServerManager() 
{
    // Stop all services
    for (auto& service : services) {
        service->stop();
    }
}

bool ServerManager::loadConfig(const std::string& configPath) 
{
    // Check if configPath is a directory (new multiple file format)
    struct stat statBuf;
    if (stat(configPath.c_str(), &statBuf) == 0 && S_ISDIR(statBuf.st_mode)) {
        // Load multiple configuration files from directory
        return loadConfigDirectory(configPath);
    } else {
        // Load single configuration file (backward compatibility)
        if (!ConfigParser::parse(configPath, config)) {
            return false;
        }
        
        // Create service instances for all services
        for (const auto& serviceConfig : config.appList) {
            auto service = std::make_shared<ServiceInstance>(serviceConfig);
            services.push_back(service);
        }
        
        for (const auto& serviceConfig : config.onStart) {
            auto service = std::make_shared<ServiceInstance>(serviceConfig);
            services.push_back(service);
        }
    }
    
    // Start on-boot services
    startOnBootServices();
    
    return true;
}

bool ServerManager::loadConfigDirectory(const std::string& configDir) 
{
    DIR* dir = opendir(configDir.c_str());
    if (!dir) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to open config directory: %s", 
            configDir.c_str());
        return false;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fileName = entry->d_name;
        if (fileName == "." || fileName == "..") {
            continue;
        }
        
        if (fileName.substr(fileName.find_last_of(".") + 1) == "json") {
            std::string filePath = configDir + "/" + fileName;
            ServiceConfig serviceConfig;
            if (ConfigParser::parseServiceFile(filePath, serviceConfig)) {
                auto service = std::make_shared<ServiceInstance>(serviceConfig, executablePath);
                services.push_back(service);
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Loaded service: %s from %s", 
                    serviceConfig.name.c_str(), fileName.c_str());
            }
        }
    }

    closedir(dir);
    return true;
}

bool ServerManager::reloadConfig() 
{
    // Clear existing services
    for (auto& service : services) {
        service->stop();
    }
    services.clear();
    
    // Reload configuration
    std::string configPath = LWComm::GetConfigPath();
    configPath = configPath + LW_OS_DIR_SEPARATOR + "server_mgr";
    return loadConfig(configPath);
}

bool ServerManager::startService(const std::string& serviceName) 
{
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto& service : services) {
        if (service->getName() == serviceName) {
            return service->start();
        }
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Service %s not found", serviceName.c_str());
    return false;
}

bool ServerManager::stopService(const std::string& serviceName) 
{
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto& service : services) {
        if (service->getName() == serviceName) {
            return service->stop();
        }
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Service %s not found", serviceName.c_str());
    return false;
}

bool ServerManager::restartService(const std::string& serviceName) 
{
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto& service : services) {
        if (service->getName() == serviceName) {
            return service->restart();
        }
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Service %s not found", serviceName.c_str());
    return false;
}

std::vector<std::string> ServerManager::listServices() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::vector<std::string> serviceNames;
    for (auto& service : services) {
        serviceNames.push_back(service->getName());
    }
    
    return serviceNames;
}

std::vector<std::tuple<std::string, bool, std::string>> ServerManager::listServicesWithStatusAndTime() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::vector<std::tuple<std::string, bool, std::string>> serviceList;
    for (auto& service : services) {
        serviceList.emplace_back(service->getName(), service->isRunning(), service->getStartTime());
    }
    
    return serviceList;
}

bool ServerManager::startAllServices() {
    std::lock_guard<std::mutex> lock(mutex);
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Starting all services...");
    
    bool allStarted = true;
    for (auto& service : services) {
        if (!service->start()) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start service: %s", 
                service->getName().c_str());
            allStarted = false;
        }
    }
    
    return allStarted;
}

void ServerManager::startOnBootServices() {
    // Start services in OnStart section
    for (const auto& serviceConfig : config.onStart) {
        auto service = std::make_shared<ServiceInstance>(serviceConfig);
        services.push_back(service);
        service->start();
    }
}
