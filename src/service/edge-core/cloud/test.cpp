#include "cloud_handler.h"
#include <iostream>
#include <csignal>

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);

    CloudHandler::Config cfg;
    cfg.brokerHost = "mqtt.example.com";
    cfg.brokerPort = 1883;
    cfg.clientId = "cpp-client";
    cfg.enableAutoReconnect = true;
    cfg.onConnect = [](int rc) {
        if (rc == MOSQ_ERR_SUCCESS) 
            std::cout << "✅ Connected to broker\n";
    };
    cfg.onDisconnect = [](int rc) {
        std::cout << "⚠️ Disconnected (code: " << rc << ")\n";
    };
    cfg.onError = [](int rc) {
        std::cerr << "❌ MQTT Error: " << mosquitto_strerror(rc) << "\n";
    };

    CloudHandler client(cfg);
    client.connect();

    // 方法1: 集成到现有事件循环
    /*
    while (running) {
        client.loop_once(100); // 100ms超时
        // 其他应用逻辑...
    }
    */

    // 方法2: 阻塞式主循环 (适合简单应用)
    /*
    std::thread loop_thread([&] {
        client.loop_forever(500); // 500ms超时
    });
    
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    client.disconnect();
    loop_thread.join();
    */

    // 方法3: 后台异步循环 (推荐)
    client.loop_async(500); // 后台线程以500ms超时运行
    
    while (running) {
        if (client.isConnected()) {
            static int count = 0;
            client.publish("sensor/data", "value=" + std::to_string(count++));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    client.stop_loop(); // 安全停止后台线程
    return 0;
}
