/*
 * CloudHandler V2 使用示例
 * 简化的单例模式设计，适合边缘计算机
 */

#include "cloud_handler_v2.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        // 1. 初始化配置
        CloudHandler::Config config;
        config.brokerHost = "broker.hivemq.com";
        config.brokerPort = 1883;
        config.clientId = "edge-device-001";
        config.keepAlive = 60;
        config.enableAutoReconnect = true;

        // 设置回调
        config.onConnect = [](int rc) {
            std::cout << "Connected with return code: " << rc << std::endl;
        };

        config.onDisconnect = [](int rc) {
            std::cout << "Disconnected with return code: " << rc << std::endl;
        };

        config.onError = [](int error_code, const std::string& error_msg) {
            std::cout << "Error " << error_code << ": " << error_msg << std::endl;
        };

        // 2. 初始化CloudHandler
        CloudHandler::initialize(config);

        // 3. 设置消息回调
        auto& mqtt = CloudHandler::getInstance();
        mqtt.setMessageCallback([](int mid, const std::string& topic, const std::string& payload) {
            std::cout << "Received message on topic '" << topic << "': " << payload << std::endl;
        });

        // 4. 连接到broker
        if (mqtt.connect()) {
            std::cout << "Connecting to broker..." << std::endl;
        }

        // 5. 启动消息循环（在独立线程中）
        mqtt.startLoop(1000); // 1秒超时

        // 6. 订阅主题
        mqtt.subscribe("edge/device/+/status", 1);

        // 7. 发布消息的示例
        std::thread publish_thread([&mqtt]() {
            for (int i = 0; i < 10; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                std::string payload = "{\"sensor\": \"temperature\", \"value\": " + std::to_string(20 + i) + "}";
                mqtt.publish("edge/device/sensor1/data", payload, 1);
                
                std::cout << "Published message: " << payload << std::endl;
            }
        });

        // 8. 保持运行
        std::cout << "MQTT client running... Press Enter to stop." << std::endl;
        std::cin.get();

        // 9. 清理
        publish_thread.join();
        mqtt.stopLoop();
        CloudHandler::shutdown();

        std::cout << "MQTT client shutdown completed." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        CloudHandler::shutdown();
        return 1;
    }

    return 0;
}