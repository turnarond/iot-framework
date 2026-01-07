// utils/time_utils.cpp
bool is_timestamp_fresh(int64_t ts, int window_sec) {
    auto now = std::time(nullptr);
    return (ts >= now - window_sec) && (ts <= now + 60); // 允许1分钟未来时间（时钟漂移）
}