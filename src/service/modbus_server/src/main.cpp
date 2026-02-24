#include "modbus_server.h"
#include <lwlog/lwlog.h>
#include <lwcomm/lwcomm.h>
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>

CLWLog g_logger;
std::atomic<bool> g_running(true);

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        g_running = false;
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Received signal %d, stopping server...", signum);
    }
}

int main() 
{
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    g_logger.SetLogFileName();
    std::string config_file_path = LWComm::GetConfigPath();
    config_file_path += LW_OS_DIR_SEPARATOR;
    config_file_path += "modbus_server.json";
    // Create Modbus server
    modbus::ModbusServer server(config_file_path);

    // Start server
    if (!server.start()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start Modbus server");
        return 1;
    }

    // Wait for signal to stop
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Modbus server running. Press Ctrl+C to stop...");
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop server
    server.stop();

    return 0;
}