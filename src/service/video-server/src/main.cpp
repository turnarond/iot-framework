#include "VideoServer.h"
#include "LogService.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>

using namespace video_server;

// Global variable to track if the server is running
volatile bool g_running = true;

// Signal handler for SIGINT and SIGTERM
void signalHandler(int signum) {
    LOG_INFO("Received signal %d, shutting down server...", signum);
    g_running = false;
}

int main(int argc, char* argv[]) {
    std::string configFile = "./config.json";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-c" && i + 1 < argc) {
            configFile = argv[i + 1];
            i++;
        } else if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            std::cout << "Usage: video-server [-c config_file]" << std::endl;
            std::cout << "  -c config_file: Specify configuration file path" << std::endl;
            std::cout << "  -h, --help: Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "Starting video server with config file: " << configFile << std::endl;
    
    // Register signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Initialize VideoServer
        auto& server = VideoServer::instance();
        
        if (!server.initialize(configFile)) {
            std::cerr << "Failed to initialize video server" << std::endl;
            return 1;
        }
        
        // Start VideoServer
        if (!server.start()) {
            std::cerr << "Failed to start video server" << std::endl;
            server.uninitialize();
            return 1;
        }
        
        std::cout << "Video server started successfully" << std::endl;
        std::cout << "Server status: " << server.getServerStatus() << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // Main loop
        while (g_running) {
            sleep(1);
        }
        
        // Stop VideoServer
        std::cout << "Stopping video server..." << std::endl;
        server.stop();
        server.uninitialize();
        
        std::cout << "Video server stopped successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}