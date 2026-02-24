#pragma once

#include <string>
#include <vector>

class Servermgrctl {
public:
    Servermgrctl();
    ~Servermgrctl();
    
    void showHelp();
    void listServices();
    void startService(const std::string& serviceName);
    void stopService(const std::string& serviceName);
    void restartService(const std::string& serviceName);
    void reloadConfig();
    
private:
    bool sendCommand(const std::string& method, const std::string& path);
    std::string getResponse();
    
    int socketfd;
};
