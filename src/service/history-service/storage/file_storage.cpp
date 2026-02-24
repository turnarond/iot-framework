/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: file_storage.cpp .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "../history_storage.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <mutex>

namespace fs = std::filesystem;

class FileStorage::Impl {
public:
    Impl() : current_file_size_(0), file_handle_(nullptr) {}
    ~Impl() {
        CloseFile();
    }

    bool Initialize(const HistoryConfig& config) {
        config_ = config;
        
        // 确保存储目录存在
        try {
            fs::create_directories(config_.storage_path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create storage directory: " << e.what() << std::endl;
            return false;
        }

        // 初始化当前文件
        return CheckAndSwitchFile();
    }

    bool Store(const HistoryRecord& record) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!file_handle_) {
            if (!CheckAndSwitchFile()) {
                return false;
            }
        }

        // 写入记录
        *file_handle_ << record.timestamp << "," 
                     << record.tag_name << "," 
                     << record.value << "," 
                     << record.driver_name << "," 
                     << record.device_name << std::endl;

        // 更新文件大小
        current_file_size_ += record.timestamp.size() + record.tag_name.size() + 
                             record.value.size() + record.driver_name.size() + 
                             record.device_name.size() + 5; // 5 for commas and newline

        // 检查文件大小
        if (current_file_size_ / (1024 * 1024) >= config_.max_file_size) {
            return CheckAndSwitchFile();
        }

        return true;
    }

    bool BatchStore(const std::vector<HistoryRecord>& records) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!file_handle_) {
            if (!CheckAndSwitchFile()) {
                return false;
            }
        }

        for (const auto& record : records) {
            *file_handle_ << record.timestamp << "," 
                         << record.tag_name << "," 
                         << record.value << "," 
                         << record.driver_name << "," 
                         << record.device_name << std::endl;

            current_file_size_ += record.timestamp.size() + record.tag_name.size() + 
                                 record.value.size() + record.driver_name.size() + 
                                 record.device_name.size() + 5;

            if (current_file_size_ / (1024 * 1024) >= config_.max_file_size) {
                if (!CheckAndSwitchFile()) {
                    return false;
                }
            }
        }

        return true;
    }

    std::vector<HistoryRecord> Query(const HistoryQueryParams& params) {
        std::vector<HistoryRecord> results;
        std::lock_guard<std::mutex> lock(mutex_);

        // 获取日期范围
        std::vector<std::string> dates = GetDateRange(params.start_time, params.end_time);

        // 遍历日期文件
        for (const auto& date : dates) {
            std::string file_path = GetFilePath(date);
            if (!fs::exists(file_path)) {
                continue;
            }

            // 读取文件
            std::ifstream file(file_path);
            if (!file.is_open()) {
                continue;
            }

            std::string line;
            while (std::getline(file, line)) {
                HistoryRecord record;
                if (ParseLine(line, record)) {
                    // 检查标签名
                    if (!params.tag_name.empty() && record.tag_name != params.tag_name) {
                        continue;
                    }

                    // 检查时间范围
                    if (!CheckTimeRange(record.timestamp, params.start_time, params.end_time)) {
                        continue;
                    }

                    results.push_back(record);

                    // 检查限制
                    if (results.size() >= params.limit) {
                        break;
                    }
                }
            }

            file.close();

            if (results.size() >= params.limit) {
                break;
            }
        }

        // 排序
        if (params.desc) {
            std::sort(results.begin(), results.end(), [](const HistoryRecord& a, const HistoryRecord& b) {
                return a.timestamp > b.timestamp;
            });
        } else {
            std::sort(results.begin(), results.end(), [](const HistoryRecord& a, const HistoryRecord& b) {
                return a.timestamp < b.timestamp;
            });
        }

        // 限制返回数量
        if (results.size() > params.limit) {
            results.resize(params.limit);
        }

        return results;
    }

    HistoryStats GetStats() {
        HistoryStats stats;
        std::lock_guard<std::mutex> lock(mutex_);

        // 遍历所有文件
        for (const auto& entry : fs::directory_iterator(config_.storage_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.size() == 10 && IsValidDate(filename)) {
                    // 计算文件大小
                    size_t file_size = entry.file_size();
                    stats.total_size += file_size;

                    // 计算记录数
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        std::string line;
                        size_t count = 0;
                        while (std::getline(file, line)) {
                            count++;
                        }
                        stats.total_records += count;

                        // 检查是否是今天的文件
                        std::string today = GetCurrentDate();
                        if (filename == today) {
                            stats.today_records = count;
                            stats.today_size = file_size;
                        }

                        file.close();
                    }

                    // 更新日期范围
                    if (stats.oldest_date.empty() || filename < stats.oldest_date) {
                        stats.oldest_date = filename;
                    }
                    if (stats.newest_date.empty() || filename > stats.newest_date) {
                        stats.newest_date = filename;
                    }
                }
            }
        }

        return stats;
    }

    size_t Cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t deleted = 0;

        // 计算过期日期
        std::time_t now = std::time(nullptr);
        std::tm* tm_now = std::localtime(&now);
        tm_now->tm_mday -= config_.max_days;
        std::time_t expire_time = std::mktime(tm_now);

        // 遍历所有文件
        for (const auto& entry : fs::directory_iterator(config_.storage_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.size() == 10 && IsValidDate(filename)) {
                    // 解析日期
                    std::tm tm_date = {};
                    std::istringstream ss(filename);
                    ss >> std::get_time(&tm_date, "%Y%m%d");
                    std::time_t file_time = std::mktime(&tm_date);

                    // 检查是否过期
                    if (file_time < expire_time) {
                        try {
                            fs::remove(entry.path());
                            deleted++;
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to delete file " << entry.path() << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
        }

        return deleted;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        CloseFile();
    }

private:
    HistoryConfig config_;
    std::string current_file_;
    size_t current_file_size_;
    std::ofstream* file_handle_;
    std::mutex mutex_;

    bool CheckAndSwitchFile() {
        CloseFile();

        // 创建新文件
        std::string date = GetCurrentDate();
        current_file_ = GetFilePath(date);
        current_file_size_ = 0;

        // 打开文件（追加模式）
        file_handle_ = new std::ofstream(current_file_, std::ios::app);
        if (!file_handle_->is_open()) {
            delete file_handle_;
            file_handle_ = nullptr;
            std::cerr << "Failed to open file: " << current_file_ << std::endl;
            return false;
        }

        return true;
    }

    void CloseFile() {
        if (file_handle_) {
            file_handle_->close();
            delete file_handle_;
            file_handle_ = nullptr;
        }
    }

    std::string GetFilePath(const std::string& date) const {
        return config_.storage_path + "/" + date + ".csv";
    }

    std::string GetCurrentDate() const {
        std::time_t now = std::time(nullptr);
        std::tm* tm_now = std::localtime(&now);
        std::ostringstream ss;
        ss << std::put_time(tm_now, "%Y%m%d");
        return ss.str();
    }

    bool ParseLine(const std::string& line, HistoryRecord& record) {
        std::istringstream ss(line);
        std::string token;

        // 解析时间戳
        if (!std::getline(ss, token, ',')) return false;
        record.timestamp = token;

        // 解析标签名
        if (!std::getline(ss, token, ',')) return false;
        record.tag_name = token;

        // 解析数值
        if (!std::getline(ss, token, ',')) return false;
        record.value = token;

        // 解析驱动名
        if (!std::getline(ss, token, ',')) return false;
        record.driver_name = token;

        // 解析设备名
        if (!std::getline(ss, token, ',')) return false;
        record.device_name = token;

        return true;
    }

    bool CheckTimeRange(const std::string& timestamp, const std::string& start_time, const std::string& end_time) {
        if (!start_time.empty() && timestamp < start_time) {
            return false;
        }
        if (!end_time.empty() && timestamp > end_time) {
            return false;
        }
        return true;
    }

    std::vector<std::string> GetDateRange(const std::string& start_time, const std::string& end_time) {
        std::vector<std::string> dates;
        std::string start_date, end_date;

        // 解析日期
        if (!start_time.empty()) {
            start_date = start_time.substr(0, 8);
        } else {
            start_date = GetOldestDate();
        }

        if (!end_time.empty()) {
            end_date = end_time.substr(0, 8);
        } else {
            end_date = GetCurrentDate();
        }

        // 生成日期范围
        std::tm tm_start = {};
        std::istringstream ss_start(start_date);
        ss_start >> std::get_time(&tm_start, "%Y%m%d");

        std::tm tm_end = {};
        std::istringstream ss_end(end_date);
        ss_end >> std::get_time(&tm_end, "%Y%m%d");

        std::time_t time_start = std::mktime(&tm_start);
        std::time_t time_end = std::mktime(&tm_end);

        for (std::time_t t = time_start; t <= time_end; t += 86400) {
            std::tm* tm_date = std::localtime(&t);
            std::ostringstream ss;
            ss << std::put_time(tm_date, "%Y%m%d");
            dates.push_back(ss.str());
        }

        return dates;
    }

    std::string GetOldestDate() const {
        std::string oldest_date;
        for (const auto& entry : fs::directory_iterator(config_.storage_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.size() == 10 && IsValidDate(filename)) {
                    if (oldest_date.empty() || filename < oldest_date) {
                        oldest_date = filename;
                    }
                }
            }
        }
        return oldest_date.empty() ? GetCurrentDate() : oldest_date;
    }

    bool IsValidDate(const std::string& date) const {
        if (date.size() != 10) {
            return false;
        }
        // 简单验证日期格式
        for (char c : date) {
            if (!std::isdigit(c)) {
                return false;
            }
        }
        return true;
    }
};

FileStorage::FileStorage() : impl_(std::make_unique<Impl>()) {}

FileStorage::~FileStorage() {}

bool FileStorage::Initialize(const HistoryConfig& config) {
    return impl_->Initialize(config);
}

bool FileStorage::Store(const HistoryRecord& record) {
    return impl_->Store(record);
}

bool FileStorage::BatchStore(const std::vector<HistoryRecord>& records) {
    return impl_->BatchStore(records);
}

std::vector<HistoryRecord> FileStorage::Query(const HistoryQueryParams& params) {
    return impl_->Query(params);
}

HistoryStats FileStorage::GetStats() {
    return impl_->GetStats();
}

size_t FileStorage::Cleanup() {
    return impl_->Cleanup();
}

void FileStorage::Close() {
    impl_->Close();
}

std::unique_ptr<HistoryStorage> CreateFileStorage() {
    return std::make_unique<FileStorage>();
}
