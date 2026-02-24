/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: dds_manager.cpp DDS communication manager implementation for NodeServer.
 *
 */

#include "dds_manager.h"
#include "lwdistcomm.h"
#include "address.h"
#include "message.h"
#include "lwlog/lwlog.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>

// External logger
extern CLWLog g_logger;

namespace nodeserver {
namespace dds {

// Static members initialization
DdsManager* DdsManager::instance_ = nullptr;
const std::string DdsManager::TOPIC_TAG_UPDATE = "/tags/update";
const std::string DdsManager::TOPIC_TAG_READ = "/tags/read";
const std::string DdsManager::TOPIC_TAG_CONTROL = "/tags/control";

/**
 * @brief Get singleton instance
 * @return DdsManager instance
 */
DdsManager* DdsManager::GetInstance() {
    if (!instance_) {
        instance_ = new DdsManager();
    }
    return instance_;
}

/**
 * @brief Private constructor (singleton)
 */
DdsManager::DdsManager() 
    : server_(nullptr),
      addr_(nullptr),
      initialized_(false),
      running_(false) {
}

/**
 * @brief Private destructor
 */
DdsManager::~DdsManager() {
    Stop();
    if (addr_) {
        lwdistcomm_address_destroy(addr_);
        addr_ = nullptr;
    }
    if (server_) {
        lwdistcomm_server_destroy(server_);
        server_ = nullptr;
    }
}

/**
 * @brief Initialize DDS manager
 * @param server_name Server name
 * @param address Address string (Unix path or IP:port)
 * @return 0 on success, negative on failure
 */
int DdsManager::Init(const std::string& server_name, const std::string& address) {
    if (initialized_) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS manager already initialized");
        return 0;
    }

    server_name_ = server_name;
    address_ = address;

    // Parse address
    if (!ParseAddress(address)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse DDS address: %s", address.c_str());
        return -1;
    }

    // Create server
    if (!CreateServer()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create DDS server");
        return -2;
    }

    initialized_ = true;
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS manager initialized successfully");
    return 0;
}

/**
 * @brief Start DDS server
 * @return 0 on success, negative on failure
 */
int DdsManager::Start() {
    if (!initialized_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DDS manager not initialized");
        return -1;
    }

    if (running_) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS server already running");
        return 0;
    }

    if (!server_ || !addr_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DDS server or address not initialized");
        return -2;
    }

    if (!lwdistcomm_server_start(server_, addr_)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start DDS server");
        return -3;
    }

    running_ = true;
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS server started successfully");
    return 0;
}

/**
 * @brief Stop DDS server
 * @return 0 on success, negative on failure
 */
int DdsManager::Stop() {
    if (!running_) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS server not running");
        return 0;
    }

    if (server_) {
        if (!lwdistcomm_server_stop(server_)) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to stop DDS server");
            return -1;
        }
    }

    running_ = false;
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS server stopped successfully");
    return 0;
}

/**
 * @brief Check if DDS server is running
 * @return true if running, false otherwise
 */
bool DdsManager::IsRunning() const {
    return running_;
}

/**
 * @brief Publish tag data
 * @param tag_data List of tag data to publish
 * @param topic Topic name
 * @return true on success, false otherwise
 */
bool DdsManager::PublishTagData(const edge_framework::dto::TagDataList& tag_data, const std::string& topic) {
    if (!running_ || !server_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DDS server not running");
        return false;
    }

    if (tag_data.empty()) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Empty tag data list");
        return false;
    }

    // Convert to JSON
    std::string json_data = TagDataListToJson(tag_data);
    if (json_data.empty()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to convert tag data to JSON");
        return false;
    }

    // Create message
    lwdistcomm_message_t msg;
    msg.data = const_cast<char*>(json_data.c_str());
    msg.data_len = json_data.length();

    // Publish message
    if (!lwdistcomm_server_publish(server_, topic.c_str(), &msg)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to publish tag data to topic: %s", topic.c_str());
        return false;
    }

    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Published %zu tag(s) to topic: %s", tag_data.size(), topic.c_str());
    return true;
}

/**
 * @brief Publish single tag data
 * @param tag_data Tag data to publish
 * @param topic Topic name
 * @return true on success, false otherwise
 */
bool DdsManager::PublishTagData(const std::shared_ptr<edge_framework::dto::TagDataDto>& tag_data, const std::string& topic) {
    if (!tag_data) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Null tag data");
        return false;
    }

    edge_framework::dto::TagDataList tag_list;
    tag_list.push_back(tag_data);
    return PublishTagData(tag_list, topic);
}

/**
 * @brief Add RPC handler for tag read
 * @param callback Handler callback
 * @param arg User argument
 * @return true on success, false otherwise
 */
bool DdsManager::AddTagReadHandler(lwdistcomm_server_handler_cb_t callback, void* arg) {
    if (!server_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DDS server not initialized");
        return false;
    }

    if (!lwdistcomm_server_add_handler(server_, TOPIC_TAG_READ.c_str(), callback, arg)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to add tag read handler");
        return false;
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Added tag read handler for topic: %s", TOPIC_TAG_READ.c_str());
    return true;
}

/**
 * @brief Add RPC handler for tag control
 * @param callback Handler callback
 * @param arg User argument
 * @return true on success, false otherwise
 */
bool DdsManager::AddTagControlHandler(lwdistcomm_server_handler_cb_t callback, void* arg) {
    if (!server_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "DDS server not initialized");
        return false;
    }

    if (!lwdistcomm_server_add_handler(server_, TOPIC_TAG_CONTROL.c_str(), callback, arg)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to add tag control handler");
        return false;
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Added tag control handler for topic: %s", TOPIC_TAG_CONTROL.c_str());
    return true;
}

/**
 * @brief Process DDS events
 * @return true on success, false otherwise
 */
bool DdsManager::ProcessEvents() {
    if (!running_ || !server_) {
        return false;
    }

    return lwdistcomm_server_process_events(server_);
}

/**
 * @brief Get file descriptors for event polling
 * @param rfds File descriptor set
 * @return Number of file descriptors
 */
int DdsManager::GetFds(fd_set* rfds) {
    if (!running_ || !server_ || !rfds) {
        return 0;
    }

    return lwdistcomm_server_get_fds(server_, rfds);
}

/**
 * @brief Process input events
 * @param rfds File descriptor set
 * @return true on success, false otherwise
 */
bool DdsManager::ProcessInput(const fd_set* rfds) {
    if (!running_ || !server_ || !rfds) {
        return false;
    }

    return lwdistcomm_server_process_input(server_, rfds);
}

/**
 * @brief Create DDS server
 * @return true on success, false otherwise
 */
bool DdsManager::CreateServer() {
    if (server_) {
        lwdistcomm_server_destroy(server_);
        server_ = nullptr;
    }

    // Create server options
    lwdistcomm_server_options_t options;
    memset(&options, 0, sizeof(options));

    // Create server
    server_ = lwdistcomm_server_create(&options);
    if (!server_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create DDS server instance");
        return false;
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS server created successfully");
    return true;
}

/**
 * @brief Parse address string
 * @param address Address string
 * @return true on success, false otherwise
 */
bool DdsManager::ParseAddress(const std::string& address) {
    if (addr_) {
        lwdistcomm_address_destroy(addr_);
        addr_ = nullptr;
    }

    // Check if it's a Unix domain socket path
    if (address.find(":") == std::string::npos) {
        // Unix domain socket
        addr_ = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_UNIX);
        if (!addr_) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create Unix address");
            return false;
        }

        if (!lwdistcomm_address_set_unix_path(addr_, address.c_str())) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to set Unix path: %s", address.c_str());
            lwdistcomm_address_destroy(addr_);
            addr_ = nullptr;
            return false;
        }

        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Using Unix domain socket: %s", address.c_str());
    } else {
        // IPv4 or IPv6 address
        size_t colon_pos = address.rfind(":");
        if (colon_pos == std::string::npos) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Invalid IP address format: %s", address.c_str());
            return false;
        }

        std::string ip = address.substr(0, colon_pos);
        std::string port_str = address.substr(colon_pos + 1);
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));

        // Check if it's IPv6
        if (ip.front() == '[' && ip.back() == ']') {
            // IPv6
            ip = ip.substr(1, ip.size() - 2);
            addr_ = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV6);
            if (!addr_) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create IPv6 address");
                return false;
            }

            if (!lwdistcomm_address_set_ipv6(addr_, ip.c_str(), port)) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to set IPv6 address: %s", ip.c_str());
                lwdistcomm_address_destroy(addr_);
                addr_ = nullptr;
                return false;
            }

            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Using IPv6 address: [%s]:%u", ip.c_str(), port);
        } else {
            // IPv4
            addr_ = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
            if (!addr_) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create IPv4 address");
                return false;
            }

            if (!lwdistcomm_address_set_ipv4(addr_, ip.c_str(), port)) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to set IPv4 address: %s", ip.c_str());
                lwdistcomm_address_destroy(addr_);
                addr_ = nullptr;
                return false;
            }

            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Using IPv4 address: %s:%u", ip.c_str(), port);
        }
    }

    return true;
}

/**
 * @brief Convert TagDataList to JSON string
 * @param tag_data List of tag data
 * @return JSON string
 */
std::string DdsManager::TagDataListToJson(const edge_framework::dto::TagDataList& tag_data) const {
    std::ostringstream json;
    json << "[";

    bool first = true;
    for (const auto& tag : tag_data) {
        if (!first) {
            json << ",";
        }
        json << TagDataToJson(tag);
        first = false;
    }

    json << "]";
    return json.str();
}

/**
 * @brief Convert single TagDataDto to JSON string
 * @param tag_data Tag data
 * @return JSON string
 */
std::string DdsManager::TagDataToJson(const std::shared_ptr<edge_framework::dto::TagDataDto>& tag_data) const {
    std::ostringstream json;
    json << "{";
    json << "\"name\":\"" << tag_data->name << "\",";
    json << "\"value\":\"" << tag_data->value << "\",";
    json << "\"time\":" << tag_data->time;

    if (!tag_data->driver_name.empty()) {
        json << ",\"driver_name\":\"" << tag_data->driver_name << "\"";
    }

    if (!tag_data->device_name.empty()) {
        json << ",\"device_name\":\"" << tag_data->device_name << "\"";
    }

    json << "}";
    return json.str();
}

} // namespace dds
} // namespace nodeserver