#ifndef MODBUS_SERVER_H
#define MODBUS_SERVER_H

#include <vector>
#include <map>
#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <string>
#include "modbus_slave.h"

namespace modbus {

class ModbusServer {
public:
    ModbusServer(const std::string& config_file);
    ~ModbusServer();

    // Start the server
    bool start();

    // Stop the server
    void stop();

    // Check if server is running
    bool is_running() const { return running_; }

private:
    // Load configuration
    bool load_config(const std::string& config_file);

    // Initialize server socket
    bool init_server_socket();

    // Server main loop
    void server_loop();

    // Handle client data
    bool handle_client_data(int client_socket);

    // Find slave by ID
    ModbusSlave* find_slave(uint8_t slave_id);

    // Configuration
    std::string config_file_;
    uint16_t port_;
    int max_connections_;
    int buffer_size_;

    // Server state
    std::atomic<bool> running_;
    int server_socket_;
    std::thread server_thread_;
    std::set<int> client_sockets_;
    std::mutex client_sockets_mutex_;

    // Slaves
    std::map<uint8_t, std::unique_ptr<ModbusSlave>> slaves_;
    std::mutex slaves_mutex_;
};

} // namespace modbus

#endif // MODBUS_SERVER_H