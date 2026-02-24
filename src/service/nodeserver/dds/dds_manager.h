/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: dds_manager.h DDS communication manager for NodeServer.
 *
 */

#pragma once

#include "lwdistcomm.h"
#include "address.h"
#include "message.h"
#include "../../common/dto/TagDataDto.hpp"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace nodeserver {
namespace dds {

/**
 * @brief DDS communication manager for NodeServer
 */
class DdsManager {
public:
    /**
     * @brief Get singleton instance
     * @return DdsManager instance
     */
    static DdsManager* GetInstance();

    /**
     * @brief Initialize DDS manager
     * @param server_name Server name
     * @param address Address string (Unix path or IP:port)
     * @return 0 on success, negative on failure
     */
    int Init(const std::string& server_name, const std::string& address);

    /**
     * @brief Start DDS server
     * @return 0 on success, negative on failure
     */
    int Start();

    /**
     * @brief Stop DDS server
     * @return 0 on success, negative on failure
     */
    int Stop();

    /**
     * @brief Check if DDS server is running
     * @return true if running, false otherwise
     */
    bool IsRunning() const;

    /**
     * @brief Publish tag data
     * @param tag_data List of tag data to publish
     * @param topic Topic name (default: "/tags/update")
     * @return true on success, false otherwise
     */
    bool PublishTagData(const edge_framework::dto::TagDataList& tag_data, const std::string& topic = "/tags/update");

    /**
     * @brief Publish single tag data
     * @param tag_data Tag data to publish
     * @param topic Topic name (default: "/tags/update")
     * @return true on success, false otherwise
     */
    bool PublishTagData(const std::shared_ptr<edge_framework::dto::TagDataDto>& tag_data, const std::string& topic = "/tags/update");

    /**
     * @brief Add RPC handler for tag read
     * @param callback Handler callback
     * @param arg User argument
     * @return true on success, false otherwise
     */
    bool AddTagReadHandler(lwdistcomm_server_handler_cb_t callback, void* arg);

    /**
     * @brief Add RPC handler for tag control
     * @param callback Handler callback
     * @param arg User argument
     * @return true on success, false otherwise
     */
    bool AddTagControlHandler(lwdistcomm_server_handler_cb_t callback, void* arg);

    /**
     * @brief Process DDS events
     * @return true on success, false otherwise
     */
    bool ProcessEvents();

    /**
     * @brief Get file descriptors for event polling
     * @param rfds File descriptor set
     * @return Number of file descriptors
     */
    int GetFds(fd_set* rfds);

    /**
     * @brief Process input events
     * @param rfds File descriptor set
     * @return true on success, false otherwise
     */
    bool ProcessInput(const fd_set* rfds);

private:
    /**
     * @brief Private constructor (singleton)
     */
    DdsManager();

    /**
     * @brief Private destructor
     */
    ~DdsManager();

    /**
     * @brief Create DDS server
     * @return true on success, false otherwise
     */
    bool CreateServer();

    /**
     * @brief Parse address string
     * @param address Address string
     * @return true on success, false otherwise
     */
    bool ParseAddress(const std::string& address);

public:
    /**
     * @brief Convert TagDataList to JSON string
     * @param tag_data List of tag data
     * @return JSON string
     */
    std::string TagDataListToJson(const edge_framework::dto::TagDataList& tag_data) const;

    /**
     * @brief Convert single TagDataDto to JSON string
     * @param tag_data Tag data
     * @return JSON string
     */
    std::string TagDataToJson(const std::shared_ptr<edge_framework::dto::TagDataDto>& tag_data) const;

private:
    static DdsManager* instance_; ///< Singleton instance
    
    std::string server_name_; ///< Server name
    std::string address_; ///< Address string
    
    lwdistcomm_server_t* server_; ///< DDS server instance
    lwdistcomm_address_t* addr_; ///< DDS address
    
    bool initialized_; ///< Initialization flag
    bool running_; ///< Running flag

    // Topic paths
    static const std::string TOPIC_TAG_UPDATE; ///< Tag update topic
    static const std::string TOPIC_TAG_READ; ///< Tag read RPC
    static const std::string TOPIC_TAG_CONTROL; ///< Tag control RPC
};

} // namespace dds
} // namespace nodeserver

#define DDS_MANAGER nodeserver::dds::DdsManager::GetInstance()