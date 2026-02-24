// services/app_management/app_manager.cpp
#include "app_manager.h"
#include "cloud/cloud_client.h"

namespace edge {
namespace services {
namespace app_management {

void AppManager::initializeCloudIntegration() {
    auto& cloudClient = cloud::CloudClient::getInstance();
    
    // 1. 订阅APP命令主题
    cloudClient.subscribe(
        "devices/" + getDeviceId() + "/apps/+/commands",
        cloud::QosLevel::AT_LEAST_ONCE,
        [this](const std::string& topic, const std::string& payload) {
            handleAppCommand(topic, payload);
        }
    );
    
    // 2. 订阅APP配置更新
    cloudClient.subscribe(
        "devices/" + getDeviceId() + "/apps/+/config",
        cloud::QosLevel::AT_LEAST_ONCE,
        [this](const std:: string& topic, const std::string& payload) {
            handleAppConfigUpdate(topic, payload);
        }
    );
    
    // 3. 发布当前APP状态
    publishAppStates();
}

void AppManager::handleAppCommand(const std::string& topic, const std::string& payload) {
    // 解析主题，提取APP ID
    std::string appId = extractAppIdFromTopic(topic);
    
    // 解析命令
    auto command = parseCommand(payload);
    
    // 直接处理命令，无需IPC
    switch (command.type) {
        case CommandType::INSTALL:
            handleInstallCommand(appId, command.params);
            break;
        case CommandType::START:
            startApp(appId);
            break;
        case CommandType::STOP:
            stopApp(appId);
            break;
        case CommandType::UPDATE:
            handleUpdateCommand(appId, command.params);
            break;
        case CommandType::UNINSTALL:
            uninstallApp(appId);
            break;
    }
    
    // 发布操作结果
    publishCommandResult(appId, command.commandId, command.type, true, "");
}

void AppManager::downloadAppFromCloud(const std::string& appId, const std::string& version) {
    // 1. 检查是否已存在
    if (appExists(appId, version)) {
        LOG_INFO("APP already exists: " + appId + " v" + version);
        return;
    }
    
    // 2. 构建下载URL
    std::string downloadUrl = buildDownloadUrl(appId, version);
    
    // 3. 使用专门的HTTP下载器（大文件优化）
    auto downloader = std::make_shared<AppDownloader>();
    
    // 4. 异步下载，不影响主流程
    downloader->downloadAsync(
        downloadUrl, 
        getAppPackagePath(appId, version),
        [this, appId, version](float progress) {
            // 更新下载进度
            updateDownloadProgress(appId, version, progress);
            
            // 定期上报进度到云端
            if (static_cast<int>(progress * 10) > static_cast<int>(m_lastReportedProgress[appId] * 10)) {
                reportDownloadProgress(appId, version, progress);
                m_lastReportedProgress[appId] = progress;
            }
        },
        [this, appId, version](bool success, const std::string& error) {
            if (success) {
                // 下载完成，验证并安装
                if (verifyAppPackage(appId, version)) {
                    installDownloadedApp(appId, version);
                    reportDownloadComplete(appId, version, true, "");
                } else {
                    reportDownloadComplete(appId, version, false, "Package verification failed");
                }
            } else {
                reportDownloadComplete(appId, version, false, error);
            }
        }
    );
    
    // 5. 保存下载器引用，防止提前销毁
    m_activeDownloaders[appId + ":" + version] = downloader;
}

void AppManager::reportDownloadProgress(const std::string& appId, const std::string& version, float progress) {
    // 直接发布到云端，无需IPC
    nlohmann::json report = {
        {"appId", appId},
        {"version", version},
        {"progress", progress},
        {"timestamp", getCurrentTimestamp()}
    };
    
    cloud::CloudClient::getInstance().publish(
        "devices/" + getDeviceId() + "/apps/" + appId + "/download/progress",
        report.dump(),
        cloud::QosLevel::AT_MOST_ONCE  // 进度可以丢失，下一条会覆盖
    );
}

} // namespace app_management
} // namespace services
} // namespace edge