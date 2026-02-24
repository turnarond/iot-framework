// ThreadedTask.h
#pragma once
#include "base_task.h"
#include <future>
#include <functional>

class ThreadedTask : public BaseTask {
public:
    using TaskFunc = std::function<void()>;  // 定义任务函数类型

    explicit ThreadedTask(const std::string& name, TaskFunc func)
        : BaseTask(name), m_func(std::move(func)) {}  // 初始化任务函数

    void start() override {
        if (isRunning()) return;

        m_future = std::async(std::launch::async, [this]() {
            m_running = true;
            m_func();  // 执行用户提供的任务函数
            m_running = false;
        });
    }

    void stop() override {
        // 无法强制停止异步任务，只能等待其完成
        if (m_future.valid()) {
            m_future.wait();  // 等待任务完成
        }
    }

private:
    TaskFunc m_func;            // 用户提供的任务函数
    std::future<void> m_future; // 异步任务句柄
};