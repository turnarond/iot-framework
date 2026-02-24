// main.cpp
#include "task_manager.h"
#include "periodic_task.h"
#include "thread_task.h"
#include <chrono>
#include <thread>

int main() 
{
    auto& tm = TaskManager::getInstance();

    // 创建一个周期任务（每秒打印一次）
    auto periodicTask = tm.createTask<PeriodicTask>("定时打印任务", []() {
        std::cout << "[定时任务] Hello from periodic task!" << std::endl;
    }, std::chrono::seconds(1));

    // 创建一个异步任务（执行完自动退出）
    auto threadedTask = tm.createTask<ThreadedTask>("异步任务", []() {
        std::cout << "[异步任务] 开始执行..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "[异步任务] 完成！" << std::endl;
    });

    tm.startAll();

    // 运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(5));

    tm.stopAll();

    return 0;
}