#include "modbus_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <lwlog/lwlog.h>

extern CLWLog g_logger;
namespace modbus {

using json = nlohmann::json;

ModbusServer::ModbusServer(const std::string& config_file)
    : config_file_(config_file),
      port_(502),
      max_connections_(100),
      buffer_size_(1024),
      running_(false),
      server_socket_(-1) {
}

ModbusServer::~ModbusServer() {
    stop();
}

bool ModbusServer::start() {
    // Load configuration
    if (!load_config(config_file_)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load configuration");
        return false;
    }

    // Initialize server socket
    if (!init_server_socket()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize server socket");
        return false;
    }

    // Start server thread
    running_ = true;
    server_thread_ = std::thread(&ModbusServer::server_loop, this);

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Modbus TCP server started on port %d", port_);
    return true;
}

void ModbusServer::stop() {
    if (running_) {
        running_ = false;

        // Close server socket to unblock select()
        if (server_socket_ != -1) {
            close(server_socket_);
            server_socket_ = -1;
        }

        // Close all client sockets
        {
            std::lock_guard<std::mutex> lock(client_sockets_mutex_);
            for (int client_socket : client_sockets_) {
                close(client_socket);
            }
            client_sockets_.clear();
        }

        // Join server thread
        if (server_thread_.joinable()) {
            server_thread_.join();
        }

        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Modbus TCP server stopped");
    }
}

bool ModbusServer::load_config(const std::string& config_file) {
    try {
        std::ifstream ifs(config_file);
        if (!ifs.is_open()) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to open config file: %s", config_file.c_str());
            return false;
        }

        json j;
        ifs >> j;

        // Load server config
        if (j.contains("server")) {
            if (j["server"].contains("port")) {
                port_ = j["server"]["port"];
            }
            if (j["server"].contains("max_connections")) {
                max_connections_ = j["server"]["max_connections"];
            }
            if (j["server"].contains("buffer_size")) {
                buffer_size_ = j["server"]["buffer_size"];
            }
        }

        // Load slaves config
        if (j.contains("slaves")) {
            for (const auto& slave_config : j["slaves"]) {
                uint8_t slave_id = slave_config["slave_id"];
                size_t coils_count = slave_config["coils_count"];
                size_t discrete_inputs_count = slave_config["discrete_inputs_count"];
                size_t holding_registers_count = slave_config["holding_registers_count"];
                size_t input_registers_count = slave_config["input_registers_count"];

                slaves_[slave_id] = std::make_unique<ModbusSlave>(
                    slave_id,
                    coils_count,
                    discrete_inputs_count,
                    holding_registers_count,
                    input_registers_count
                );

                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Added slave %d with %zu coils, %zu discrete inputs, %zu holding registers, %zu input registers",
                           slave_id, coils_count, discrete_inputs_count, holding_registers_count, input_registers_count);
            }
        }

        return true;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Error loading config: %s", e.what());
        return false;
    }
}

bool ModbusServer::init_server_socket() {
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create socket: %s", strerror(errno));
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to set socket options: %s", strerror(errno));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    // Bind socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to bind socket: %s", strerror(errno));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    // Listen for connections
    if (listen(server_socket_, max_connections_) < 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to listen: %s", strerror(errno));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    return true;
}

void ModbusServer::server_loop() {
    // Add server socket to client sockets list
    {
        std::lock_guard<std::mutex> lock(client_sockets_mutex_);
        client_sockets_.insert(server_socket_);
    }

    while (running_) {
        fd_set read_fds;
        FD_ZERO(&read_fds);

        // Find the maximum file descriptor
        int max_fd = server_socket_;

        // Add all client sockets to the set
        {
            std::lock_guard<std::mutex> lock(client_sockets_mutex_);
            for (int client_socket : client_sockets_) {
                FD_SET(client_socket, &read_fds);
                if (client_socket > max_fd) {
                    max_fd = client_socket;
                }
            }
        }

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;

        // Wait for activity
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready < 0) {
            if (errno != EINTR) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Select failed: %s", strerror(errno));
            }
            continue;
        }

        if (ready == 0) {
            // Timeout, continue to check running flag
            continue;
        }

        // Check server socket for new connections
        if (FD_ISSET(server_socket_, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);

            int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_socket < 0) {
                if (running_) {
                    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to accept connection: %s", strerror(errno));
                }
                continue;
            }

            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Accepted connection from %s", inet_ntoa(client_addr.sin_addr));

            // Set socket to non-blocking
            int flags = fcntl(client_socket, F_GETFL, 0);
            fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

            // Add to client sockets list
            {
                std::lock_guard<std::mutex> lock(client_sockets_mutex_);
                client_sockets_.insert(client_socket);
            }
        }

        // Check client sockets for data
        {
            std::lock_guard<std::mutex> lock(client_sockets_mutex_);
            auto it = client_sockets_.begin();
            while (it != client_sockets_.end()) {
                int client_socket = *it;
                if (client_socket != server_socket_ && FD_ISSET(client_socket, &read_fds)) {
                    // Handle client data
                    if (!handle_client_data(client_socket)) {
                        // Connection closed or error, remove from list
                        close(client_socket);
                        it = client_sockets_.erase(it);
                        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Client connection closed");
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
        }
    }
}

bool ModbusServer::handle_client_data(int client_socket) {
    std::vector<uint8_t> buffer(buffer_size_);

    // Receive data
    ssize_t bytes_received = recv(client_socket, buffer.data(), buffer.size(), 0);
    if (bytes_received <= 0) {
        if (bytes_received < 0 && errno != EWOULDBLOCK) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Recv failed: %s", strerror(errno));
        }
        return false;
    }

    // Parse Modbus request
    ModbusRequest request;
    if (!parse_modbus_request(std::vector<uint8_t>(buffer.begin(), buffer.begin() + bytes_received), request)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse Modbus request");
        return true;  // Don't close connection for parse errors
    }

    // Find slave
    ModbusSlave* slave = find_slave(request.header.unit_id);
    if (!slave) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Slave not found: %d", request.header.unit_id);
        std::vector<uint8_t> error_response = build_modbus_error_response(request, ExceptionCode::SLAVE_DEVICE_FAILURE);
        // Use non-blocking send
        send(client_socket, error_response.data(), error_response.size(), MSG_DONTWAIT);
        return true;
    }

    // Handle request
    std::vector<uint8_t> response = slave->handle_request(request);

    // Send response with non-blocking flag
    send(client_socket, response.data(), response.size(), MSG_DONTWAIT);
    return true;
}

ModbusSlave* ModbusServer::find_slave(uint8_t slave_id) {
    std::lock_guard<std::mutex> lock(slaves_mutex_);
    auto it = slaves_.find(slave_id);
    if (it != slaves_.end()) {
        return it->second.get();
    }
    return nullptr;
}

} // namespace modbus