#pragma once
#include <string>
#include "aco/config.hpp"

namespace aco::server {

class HttpServer {
public:
    explicit HttpServer(const ServerConfig& config);
    void run();

private:
    ServerConfig config_;
};

} // namespace aco::server