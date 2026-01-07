#pragma once
#include <string>

namespace aco {

struct ServerConfig {
    std::string host = "0.0.0.0";
    uint16_t port = 8080;
    std::string db_path = "registry.db";
};

ServerConfig load_config(); // 简化：硬编码或读环境变量

} // namespace aco