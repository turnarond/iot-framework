/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: test_dds_client.cpp Test client for DDS communication.
 *
 */

#include "dds/dds_manager.h"
#include "../../common/dto/TagDataDto.hpp"
#include "lwlog/lwlog.h"

#include <lwdistcomm.h>
#include <address.h>
#include <message.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Logger definition
CLWLog g_logger;

// DDS client instance
lwdistcomm_client_t* client = nullptr;

// Subscription callback
static void subscriptionCallback(void *arg, const char *url, const lwdistcomm_message_t *msg)
{
    std::string msg_str(static_cast<const char*>(msg->data), msg->data_len);
    std::cout << "[Subscription] Received message from topic '" << url << "': " << msg_str << std::endl;
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS subscription callback: topic=%s, message=%s", url, msg_str.c_str());
}

// RPC response callback
static void rpcResponseCallback(void *arg, int status, const lwdistcomm_message_t *resp)
{
    std::string resp_str(static_cast<const char*>(resp->data), resp->data_len);
    std::cout << "[RPC Response] Received: " << resp_str << std::endl;
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "DDS RPC response: %s", resp_str.c_str());
}

// Initialize DDS client
bool initDdsClient(const std::string& address)
{
    // Create client options
    lwdistcomm_client_options_t options;
    memset(&options, 0, sizeof(options));

    // Create client
    client = lwdistcomm_client_create(&options);
    if (!client) {
        std::cerr << "Failed to create DDS client" << std::endl;
        return false;
    }

    // Create address
    lwdistcomm_address_t* addr = nullptr;
    
    // Check if it's a Unix domain socket path
    if (address.find(":") == std::string::npos) {
        // Unix domain socket
        addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_UNIX);
        if (!addr) {
            std::cerr << "Failed to create Unix address" << std::endl;
            lwdistcomm_client_destroy(client);
            client = nullptr;
            return false;
        }

        if (!lwdistcomm_address_set_unix_path(addr, address.c_str())) {
            std::cerr << "Failed to set Unix path: " << address << std::endl;
            lwdistcomm_address_destroy(addr);
            lwdistcomm_client_destroy(client);
            client = nullptr;
            return false;
        }
    } else {
        // IPv4 or IPv6 address
        size_t colon_pos = address.rfind(":");
        if (colon_pos == std::string::npos) {
            std::cerr << "Invalid IP address format: " << address << std::endl;
            lwdistcomm_client_destroy(client);
            client = nullptr;
            return false;
        }

        std::string ip = address.substr(0, colon_pos);
        std::string port_str = address.substr(colon_pos + 1);
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));

        // Check if it's IPv6
        if (ip.front() == '[' && ip.back() == ']') {
            // IPv6
            ip = ip.substr(1, ip.size() - 2);
            addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV6);
            if (!addr) {
                std::cerr << "Failed to create IPv6 address" << std::endl;
                lwdistcomm_client_destroy(client);
                client = nullptr;
                return false;
            }

            if (!lwdistcomm_address_set_ipv6(addr, ip.c_str(), port)) {
                std::cerr << "Failed to set IPv6 address: " << ip << std::endl;
                lwdistcomm_address_destroy(addr);
                lwdistcomm_client_destroy(client);
                client = nullptr;
                return false;
            }
        } else {
            // IPv4
            addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
            if (!addr) {
                std::cerr << "Failed to create IPv4 address" << std::endl;
                lwdistcomm_client_destroy(client);
                client = nullptr;
                return false;
            }

            if (!lwdistcomm_address_set_ipv4(addr, ip.c_str(), port)) {
                std::cerr << "Failed to set IPv4 address: " << ip << std::endl;
                lwdistcomm_address_destroy(addr);
                lwdistcomm_client_destroy(client);
                client = nullptr;
                return false;
            }
        }
    }

    // Connect to server
    if (!lwdistcomm_client_connect(client, addr)) {
        std::cerr << "Failed to connect to DDS server" << std::endl;
        lwdistcomm_address_destroy(addr);
        lwdistcomm_client_destroy(client);
        client = nullptr;
        return false;
    }

    // Destroy address
    lwdistcomm_address_destroy(addr);

    std::cout << "DDS client connected successfully" << std::endl;
    return true;
}

// Subscribe to topic
bool subscribeToTopic(const std::string& topic)
{
    if (!client) {
        std::cerr << "DDS client not initialized" << std::endl;
        return false;
    }

    if (!lwdistcomm_client_subscribe(client, topic.c_str(), subscriptionCallback, nullptr)) {
        std::cerr << "Failed to subscribe to topic '" << topic << "'" << std::endl;
        return false;
    }

    std::cout << "Subscribed to topic '" << topic << "'" << std::endl;
    return true;
}

// Send RPC request
bool sendRpcRequest(const std::string& url, const std::string& request)
{
    if (!client) {
        std::cerr << "DDS client not initialized" << std::endl;
        return false;
    }

    // Create message
    lwdistcomm_message_t msg;
    msg.data = const_cast<char*>(request.c_str());
    msg.data_len = request.length();

    // Send RPC request
    if (!lwdistcomm_client_rpc(client, url.c_str(), &msg, rpcResponseCallback, nullptr)) {
        std::cerr << "Failed to send RPC request to '" << url << "'" << std::endl;
        return false;
    }

    std::cout << "Sent RPC request to '" << url << "': " << request << std::endl;
    return true;
}

// Process client events
void processEvents()
{
    while (true) {
        if (client) {
            lwdistcomm_client_process_events(client);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Cleanup
void cleanup()
{
    if (client) {
        lwdistcomm_client_destroy(client);
        client = nullptr;
    }
    std::cout << "DDS client cleanup completed" << std::endl;
}

int main()
{
    std::cout << "=== DDS Client Test ===" << std::endl;

    // DDS server address
    std::string dds_address = "/tmp/nodeserver_dds.sock";

    // Initialize DDS client
    if (!initDdsClient(dds_address)) {
        std::cerr << "Failed to initialize DDS client" << std::endl;
        return 1;
    }

    // Start event processing thread
    std::thread eventThread(processEvents);

    // Wait a bit for connection to establish
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Subscribe to tag updates
    if (!subscribeToTopic("/tags/update")) {
        std::cerr << "Failed to subscribe to tag updates" << std::endl;
        cleanup();
        eventThread.detach();
        return 1;
    }

    // Test 1: Read tags via RPC
    std::cout << "\n--- Test 1: Read Tags ---" << std::endl;
    std::string read_request = "[\"temperature\", \"humidity\", \"pressure\"]";
    sendRpcRequest("/tags/read", read_request);

    // Wait for response
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test 2: Control tag via RPC
    std::cout << "\n--- Test 2: Control Tag ---" << std::endl;
    std::string control_request = "{\"name\": \"fan_speed\", \"value\": \"medium\"}";
    sendRpcRequest("/tags/control", control_request);

    // Wait for response
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Keep running to receive subscriptions
    std::cout << "\n--- Waiting for subscriptions (press Ctrl+C to exit) ---" << std::endl;

    try {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Cleanup
    cleanup();
    eventThread.detach();

    return 0;
}
