#include "websocket_server.hpp"
#include "rtdb.hpp"
#include "lwlog/lwlog.h"
#include <boost/asio/strand.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>
#include <iostream>

extern CLWLog g_logger;

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace asio = boost::asio;

namespace nodeserver {

// forward declare Impl for Session
struct WebSocketServer::Impl;

// Session is a concrete type used by Impl; define it here so Impl methods can use it
struct Session : std::enable_shared_from_this<Session> {
    websocket::stream<tcp::socket> ws;
    std::mutex write_mutex;
    std::set<std::string> subscriptions; // prefix subscriptions
    WebSocketServer::Impl* owner;
    beast::flat_buffer buffer;

    Session(tcp::socket&& sock, WebSocketServer::Impl* o)
        : ws(std::move(sock)), owner(o) {
    }

    void start() {
        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
        ws.async_accept([self = shared_from_this()](boost::system::error_code ec){
            if (ec) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[WebSocketServer] Accept failed: %s", ec.message().c_str());
                return;
            }
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Connection accepted");
            self->do_read();
        });
    }

    void do_read() {
        auto self = shared_from_this();
        ws.async_read(buffer, [self](boost::system::error_code ec, std::size_t bytes){
            if (ec) return; // connection closed or error
            std::string msg = beast::buffers_to_string(self->buffer.data());
            self->buffer.consume(self->buffer.size());
            self->handle_message(msg);
            self->do_read();
        });
    }

    void handle_message(const std::string& msg) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Received message: %s", msg.c_str());
        
        // simple text protocol: SUBSCRIBE <prefix> or UNSUBSCRIBE <prefix>
        if (msg.rfind("SUBSCRIBE ", 0) == 0) {
            std::string pref = msg.substr(10);
            if (!pref.empty()) {
                subscriptions.insert(pref);
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Subscribed to prefix: %s", pref.c_str());
            }
        } else if (msg.rfind("UNSUBSCRIBE ", 0) == 0) {
            std::string pref = msg.substr(12);
            if (!pref.empty()) {
                subscriptions.erase(pref);
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Unsubscribed from prefix: %s", pref.c_str());
            }
        } else if (msg == "PING") {
            send_text("PONG");
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Sent PONG response");
        } else {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[WebSocketServer] Ignoring unknown message: %s", msg.c_str());
        }
    }

    void send_text(const std::string& s) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Sending text: %s", s.c_str());
        
        std::lock_guard<std::mutex> lock(write_mutex);
        boost::system::error_code ec;
        ws.text(true);
        ws.write(asio::buffer(s), ec);
        
        if (ec) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[WebSocketServer] Send failed: %s", ec.message().c_str());
        }
    }

    void send_update(const TagRecord& rec) {
        // check subscriptions
        bool match = false;
        for (const auto &p : subscriptions) {
            if (rec.name.rfind(p, 0) == 0) { match = true; break; }
        }
        if (!match) return;
        
        // simple serialization: JSON-lite
        std::string out = "{\"event\":\"POINT_UPDATE\",\"name\":\"" + rec.name + "\",\"value\":" + rec.value + ",\"timestamp\":" + std::to_string(rec.timestamp_ms) + ",\"driver\":\"" + rec.driver_name + "\",\"device\":\"" + rec.device_name + "\"}";
        
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Sending point update: %s", out.c_str());
        
        // send asynchronously but protected by mutex to avoid concurrent writes
        std::lock_guard<std::mutex> lock(write_mutex);
        boost::system::error_code ec;
        ws.text(true);
        ws.write(asio::buffer(out), ec);
        
        if (ec) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[WebSocketServer] Send update failed: %s", ec.message().c_str());
        }
    }
};

struct WebSocketServer::Impl {
    asio::io_context ioc;
    std::unique_ptr<asio::io_context::work> work_guard;
    std::thread io_thread;
    std::shared_ptr<tcp::acceptor> acceptor;

    // sessions management
    std::mutex sessions_mutex;
    std::vector<std::weak_ptr<Session>> sessions;

    Impl(): ioc(1), work_guard(std::make_unique<asio::io_context::work>(ioc)) {}
    ~Impl(){ stop(); }

    void start_accept(unsigned short port) {
        try {
            acceptor = std::make_shared<tcp::acceptor>(ioc, tcp::endpoint(tcp::v4(), port));
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Started on port %u", port);
            do_accept();
            io_thread = std::thread([this]{ try { ioc.run(); } catch(...) {} });
        } catch (const std::exception& e) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[WebSocketServer] Failed to start: %s", e.what());
        }
    }

    void stop() {
        try {
            if (acceptor) acceptor->close();
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Stopped");
        } catch(...) {} 
        work_guard.reset();
        ioc.stop();
        if (io_thread.joinable()) io_thread.join();
        // clear sessions
        std::lock_guard<std::mutex> lock(sessions_mutex);
        sessions.clear();
    }

    void do_accept() {
        if (!acceptor) return;
        acceptor->async_accept([this](boost::system::error_code ec, tcp::socket socket){
            if (!ec) {
                auto remote_endpoint = socket.remote_endpoint();
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] New client connected from %s:%u", 
                    remote_endpoint.address().to_string().c_str(), remote_endpoint.port());
                
                auto s = std::make_shared<Session>(std::move(socket), this);
                {
                    std::lock_guard<std::mutex> lock(sessions_mutex);
                    sessions.push_back(s);
                }
                s->start();
            } else {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[WebSocketServer] Accept error: %s", ec.message().c_str());
            }
            // accept next
            do_accept();
        });
    }

    void broadcast_update(const TagRecord& rec) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Broadcasting point update for %s", rec.name.c_str());
        
        std::vector<std::shared_ptr<Session>> list;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex);
            for (auto it = sessions.begin(); it != sessions.end(); ) {
                if (auto sp = it->lock()) { list.push_back(sp); ++it; }
                else it = sessions.erase(it);
            }
        }
        
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Broadcasting to %zu sessions", list.size());
        for (auto &s : list) s->send_update(rec);
    }
};

WebSocketServer::WebSocketServer() : impl_(new Impl()) {}
WebSocketServer::~WebSocketServer(){ stop(); }

bool WebSocketServer::start(unsigned short port) {
    try {
        impl_->start_accept(port);
        return true;
    } catch(...) { return false; }
}

void WebSocketServer::stop() { impl_->stop(); }

void WebSocketServer::broadcastPointUpdate(const TagRecord& rec) { impl_->broadcast_update(rec); }

} // namespace nodeserver
