#pragma once
#include <string>
#include <optional>
#include "aco/result.hpp"

namespace aco::storage {

struct DeviceRecord {
    int id = -1;
    std::string serial_number;
    std::string model;
    std::string owner;
    std::string public_key_pem;
    std::string status = "PENDING";
};

class DeviceRepository {
public:
    static Result<void> init(const std::string& db_path);
    static Result<void> pre_register(const DeviceRecord& dev);
    static Result<std::optional<DeviceRecord>> get_by_sn(std::string_view sn);
};

} // namespace aco::storage