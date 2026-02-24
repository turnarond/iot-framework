// TaskManager.h
#pragma once
#include "task.h"
#include <vector>
#include <memory>

#include <iostream>

class TaskManager {
public:
    static TaskManager& getInstance() {
        static TaskManager instance;
        return instance;
    }

    template<typename TaskType, typename... Args>
    std::shared_ptr<Task> createTask(Args&&... args) {
        auto task = std::make_shared<TaskType>(std::forward<Args>(args)...);
        m_tasks.push_back(task);
        return task;
    }

    void startAll() {
        for (auto& task : m_tasks) {
            task->start();
        }
    }

    void stopAll() {
        for (auto& task : m_tasks) {
            task->stop();
        }
    }

    void listTasks() const {
        std::cout << "当前任务列表：" << std::endl;
        for (const auto& task : m_tasks) {
            std::cout << " - " << task->name() << " (" << (task->isRunning() ? "运行中" : "已停止") << ")" << std::endl;
        }
    }

private:
    TaskManager() = default;
    ~TaskManager() {
        stopAll();
    }

    std::vector<std::shared_ptr<Task>> m_tasks;
};
