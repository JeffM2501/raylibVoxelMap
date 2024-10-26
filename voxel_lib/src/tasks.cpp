#include "tasks.h"

#include "BS_thread_pool.hpp"

namespace Tasks
{
    BS::thread_pool* ThreadPool = nullptr;

    void Init()
    {
        ThreadPool = new BS::thread_pool();
    }

    bool AddTask(TaskFunction task)
    {
        if (ThreadPool == nullptr)
            return false;

        ThreadPool->detach_task([task]() {task(); });
        return true;
    }
    
    void Shutdown()
    {
        if (ThreadPool)
        {
            ThreadPool->purge();
            ThreadPool->wait();

            delete(ThreadPool);
            ThreadPool = nullptr;
        }
    }
}