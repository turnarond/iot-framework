// PeriodicTask.h
#pragma once
#include "base_task.h"
#include <chrono>
#include <functional>
#include <future>
#include <thread>

class PeriodicTask : public BaseTask {
public:
    using TaskFunc = std::function<void()>;

    PeriodicTask(const std::string& name, TaskFunc func, std::chrono::milliseconds interval)
        : BaseTask(name), m_func(std::move(func)), m_interval(interval) {}

    void start() override {
        if (isRunning()) return;

        m_running = true;
        m_worker = std::thread([this]() {
            while (m_running) {
                m_func();
                std::this_thread::sleep_for(m_interval);
            }
        });
    }

    void stop() override {
        m_running = false;
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

private:
    TaskFunc m_func;
    std::chrono::milliseconds m_interval;
    std::thread m_worker;
};