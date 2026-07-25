#include <cippie/build/ThreadPool.hpp>

namespace cippie
{
    ThreadPool::ThreadPool(size_t workerCount)
    {
        if (workerCount == 0)
        {
            workerCount = 1;
        }

        m_workers.reserve(workerCount);
        for (size_t i = 0; i < workerCount; ++i)
        {
            m_workers.emplace_back([this]() {
                for (;;)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_cv.wait(lock, [this]() {
                            return m_stop || !m_tasks.empty();
                        });

                        if (m_stop && m_tasks.empty())
                        {
                            return;
                        }

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();

        for (auto& worker : m_workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    void ThreadPool::enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks.push(std::move(task));
        }
        m_cv.notify_one();
    }
}
