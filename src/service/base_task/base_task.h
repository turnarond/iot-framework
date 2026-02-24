// BaseTask.h
#pragma once
#include "task.h"
#include <string>
#include <atomic>
#include <mutex>

class BaseTask : public Task {
public:
    explicit BaseTask(const std::string& name)
        : m_name(name), m_running(false) {}

    std::string name() const override {
        return m_name;
    }

    bool isRunning() const override {
        return m_running;
    }

protected:
    std::string m_name;
    std::atomic<bool> m_running;
    mutable std::mutex m_mutex;
};