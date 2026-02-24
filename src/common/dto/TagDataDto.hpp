/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: TagDataDto.hpp Common tag data DTO for all services.
 *
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace edge_framework {
namespace dto {

/**
 * @brief Tag data DTO for point data exchange
 */
class TagDataDto {
public:
    std::string name;        ///< Tag name
    std::string value;       ///< Tag value
    uint64_t time;          ///< Timestamp in milliseconds
    std::string driver_name; ///< Driver name
    std::string device_name; ///< Device name

    /**
     * @brief Default constructor
     */
    TagDataDto() : time(0) {
    }

    /**
     * @brief Constructor with basic parameters
     * @param name Tag name
     * @param value Tag value
     * @param time Timestamp in milliseconds
     */
    TagDataDto(const std::string& name, const std::string& value, uint64_t time)
        : name(name), value(value), time(time) {
    }

    /**
     * @brief Constructor with all parameters
     * @param name Tag name
     * @param value Tag value
     * @param time Timestamp in milliseconds
     * @param driver_name Driver name
     * @param device_name Device name
     */
    TagDataDto(const std::string& name, const std::string& value, uint64_t time,
               const std::string& driver_name, const std::string& device_name)
        : name(name), value(value), time(time),
          driver_name(driver_name), device_name(device_name) {
    }

    /**
     * @brief Create shared pointer instance
     * @return Shared pointer to TagDataDto
     */
    static std::shared_ptr<TagDataDto> createShared() {
        return std::make_shared<TagDataDto>();
    }

    /**
     * @brief Create shared pointer instance with parameters
     * @param name Tag name
     * @param value Tag value
     * @param time Timestamp in milliseconds
     * @return Shared pointer to TagDataDto
     */
    static std::shared_ptr<TagDataDto> createShared(const std::string& name, const std::string& value, uint64_t time) {
        return std::make_shared<TagDataDto>(name, value, time);
    }
};

/**
 * @brief List of TagDataDto for batch operations
 */
typedef std::vector<std::shared_ptr<TagDataDto>> TagDataList;

/**
 * @brief Control command DTO
 */
class ControlCommandDto {
public:
    std::string name;        ///< Tag name to control
    std::string value;       ///< Target value

    /**
     * @brief Default constructor
     */
    ControlCommandDto() {
    }

    /**
     * @brief Constructor with parameters
     * @param name Tag name
     * @param value Target value
     */
    ControlCommandDto(const std::string& name, const std::string& value)
        : name(name), value(value) {
    }

    /**
     * @brief Create shared pointer instance
     * @return Shared pointer to ControlCommandDto
     */
    static std::shared_ptr<ControlCommandDto> createShared() {
        return std::make_shared<ControlCommandDto>();
    }

    /**
     * @brief Create shared pointer instance with parameters
     * @param name Tag name
     * @param value Target value
     * @return Shared pointer to ControlCommandDto
     */
    static std::shared_ptr<ControlCommandDto> createShared(const std::string& name, const std::string& value) {
        return std::make_shared<ControlCommandDto>(name, value);
    }
};

} // namespace dto
} // namespace edge_framework