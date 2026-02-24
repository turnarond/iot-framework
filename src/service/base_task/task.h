// Task.h
#pragma once
#include <string>
#include <functional>
#include <memory>

class Task {
public:
    virtual ~Task() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool isRunning() const = 0;
    virtual std::string name() const = 0;
};