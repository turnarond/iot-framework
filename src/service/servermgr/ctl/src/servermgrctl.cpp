#include "servermgrctl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

Servermgrctl::Servermgrctl() {
    // Create socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    // Connect to servermgr
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8082);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(socketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(socketfd);
        throw std::runtime_error("Failed to connect to servermgr. Make sure it's running.");
    }
}

Servermgrctl::~Servermgrctl() {
    if (socketfd >= 0) {
        close(socketfd);
    }
}

void Servermgrctl::showHelp() {
    std::cout << "servermgrctl - Control utility for servermgr" << std::endl;
    std::cout << "Usage: servermgrctl <command> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  help                Show this help message" << std::endl;
    std::cout << "  list                List all services" << std::endl;
    std::cout << "  start <service>     Start a service" << std::endl;
    std::cout << "  stop <service>      Stop a service" << std::endl;
    std::cout << "  restart <service>   Restart a service" << std::endl;
    std::cout << "  reload              Reload configuration" << std::endl;
}

void Servermgrctl::listServices() {
    if (sendCommand("GET", "/services")) {
        std::string response = getResponse();
        std::cout << "Services:" << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "| Service Name      | Status    | Start Time          |" << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
        
        // Parse JSON response manually
        // Simple parsing for the expected format: [{"name":"...","running":true,"status":"...","startTime":"..."}, ...]
        size_t start = response.find('[');
        size_t end = response.find(']');
        if (start != std::string::npos && end != std::string::npos) {
            std::string arrayContent = response.substr(start + 1, end - start - 1);
            size_t pos = 0;
            while (pos < arrayContent.size()) {
                size_t objStart = arrayContent.find('{', pos);
                if (objStart == std::string::npos) break;
                size_t objEnd = arrayContent.find('}', objStart);
                if (objEnd == std::string::npos) break;
                
                std::string objContent = arrayContent.substr(objStart + 1, objEnd - objStart - 1);
                
                // Extract name
                std::string name = "";
                size_t nameStart = objContent.find("\"name\":\"");
                if (nameStart != std::string::npos) {
                    size_t nameValueStart = nameStart + 8;
                    size_t nameValueEnd = objContent.find('"', nameValueStart);
                    if (nameValueEnd != std::string::npos) {
                        name = objContent.substr(nameValueStart, nameValueEnd - nameValueStart);
                    }
                }
                
                // Extract status
                std::string status = "";
                size_t statusStart = objContent.find("\"status\":\"");
                if (statusStart != std::string::npos) {
                    size_t statusValueStart = statusStart + 10;
                    size_t statusValueEnd = objContent.find('"', statusValueStart);
                    if (statusValueEnd != std::string::npos) {
                        status = objContent.substr(statusValueStart, statusValueEnd - statusValueStart);
                    }
                }
                
                // Extract start time
                std::string startTime = "";
                size_t timeStart = objContent.find("\"startTime\":\"");
                if (timeStart != std::string::npos) {
                    size_t timeValueStart = timeStart + 14;
                    size_t timeValueEnd = objContent.find('"', timeValueStart);
                    if (timeValueEnd != std::string::npos) {
                        startTime = objContent.substr(timeValueStart, timeValueEnd - timeValueStart);
                    }
                }
                
                // Print the row
                if (!name.empty()) {
                    std::cout << "| " << std::left << std::setw(17) << name 
                              << " | " << std::left << std::setw(8) << status 
                              << " | " << std::left << std::setw(18) << (startTime.empty() ? "N/A" : startTime)
                              << " |" << std::endl;
                }
                
                pos = objEnd + 1;
            }
        }
        
        std::cout << "---------------------------------------------------------------" << std::endl;
    }
}

void Servermgrctl::startService(const std::string& serviceName) {
    if (sendCommand("POST", "/services/start/" + serviceName)) {
        std::string response = getResponse();
        std::cout << "Start service " << serviceName << ": " << response << std::endl;
    }
}

void Servermgrctl::stopService(const std::string& serviceName) {
    if (sendCommand("POST", "/services/stop/" + serviceName)) {
        std::string response = getResponse();
        std::cout << "Stop service " << serviceName << ": " << response << std::endl;
    }
}

void Servermgrctl::restartService(const std::string& serviceName) {
    if (sendCommand("POST", "/services/restart/" + serviceName)) {
        std::string response = getResponse();
        std::cout << "Restart service " << serviceName << ": " << response << std::endl;
    }
}

void Servermgrctl::reloadConfig() {
    if (sendCommand("POST", "/services/reload")) {
        std::string response = getResponse();
        std::cout << "Reload config: " << response << std::endl;
    }
}

bool Servermgrctl::sendCommand(const std::string& method, const std::string& path) {
    std::stringstream request;
    request << method << " " << path << " HTTP/1.1\r\n";
    request << "Host: localhost:8082\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: 0\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    
    std::string requestStr = request.str();
    if (send(socketfd, requestStr.c_str(), requestStr.size(), 0) < 0) {
        std::cerr << "Failed to send command" << std::endl;
        return false;
    }
    
    return true;
}

std::string Servermgrctl::getResponse() {
    char buffer[4096] = {0};
    int valread = read(socketfd, buffer, 4096);
    
    if (valread < 0) {
        throw std::runtime_error("Failed to read response");
    }
    
    // Parse HTTP response to get body
    std::string response(buffer, valread);
    size_t bodyStart = response.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        return response.substr(bodyStart + 4);
    }
    
    return response;
}
