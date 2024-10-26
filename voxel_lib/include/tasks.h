#pragma once

#include <functional>

namespace Tasks
{
    void Init();
    void Shutdown();

    using TaskFunction = std::function<void()>;
    bool AddTask(TaskFunction task);
}