#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace cippie
{
    class ThreadPool
    {
    public:
        explicit ThreadPool(size_t workerCount);
        ~ThreadPool();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        void enqueue(std::function<void()> task);

    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_tasks;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_stop{false};
    };
}
