#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <set>
#include <vector>
#include <mutex>

namespace nodeserver {

struct TagRecord;

class WebSocketServer {
public:
    explicit WebSocketServer();
    ~WebSocketServer();

    // start server on given port (non-blocking)
    bool start(unsigned short port);
    void stop();

    // broadcast a point update to subscribed sessions
    void broadcastPointUpdate(const TagRecord& rec);
public:
    struct Impl;
    
private:
    std::unique_ptr<Impl> impl_;
};

} // namespace nodeserver
