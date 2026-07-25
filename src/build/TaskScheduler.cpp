#include <cippie/build/TaskScheduler.hpp>
#include <cippie/build/ThreadPool.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_set>

namespace cippie
{
    bool TaskScheduler::execute(
        std::vector<ExecutionNode>& nodes,
        size_t workerCount,
        const std::function<bool(ExecutionNode& node)>& executor
    )
    {
        if (nodes.empty())
        {
            return true;
        }

        std::unordered_map<std::string, size_t> nodeIndexMap;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodeIndexMap[nodes[i].id] = i;
        }

        std::vector<size_t> unfinishedDepCount(nodes.size(), 0);
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodes[i].dependents.clear();
        }

        for (size_t i = 0; i < nodes.size(); ++i)
        {
            unfinishedDepCount[i] = nodes[i].dependencies.size();
            for (const auto& depId : nodes[i].dependencies)
            {
                auto it = nodeIndexMap.find(depId);
                if (it != nodeIndexMap.end())
                {
                    nodes[it->second].dependents.push_back(nodes[i].id);
                }
            }
        }

        std::mutex mtx;
        std::condition_variable cv;
        size_t remainingNodes = nodes.size();
        bool anyFailed = false;

        ThreadPool pool(workerCount);

        auto markBlocked = [&](auto& self, size_t idx) -> void {
            if (nodes[idx].state == NodeState::pending || nodes[idx].state == NodeState::ready)
            {
                nodes[idx].state = NodeState::blocked;
                remainingNodes--;

                for (const auto& depId : nodes[idx].dependents)
                {
                    auto it = nodeIndexMap.find(depId);
                    if (it != nodeIndexMap.end())
                    {
                        self(self, it->second);
                    }
                }
            }
        };

        auto dispatchNode = [&](auto& self, size_t idx) -> void {
            pool.enqueue([&, idx]() {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (nodes[idx].state == NodeState::blocked)
                    {
                        return;
                    }
                    nodes[idx].state = NodeState::running;
                }

                bool success = executor(nodes[idx]);

                std::lock_guard<std::mutex> lock(mtx);
                if (success)
                {
                    if (nodes[idx].state != NodeState::cached)
                    {
                        nodes[idx].state = NodeState::succeeded;
                    }
                    remainingNodes--;

                    for (const auto& depId : nodes[idx].dependents)
                    {
                        auto it = nodeIndexMap.find(depId);
                        if (it != nodeIndexMap.end())
                        {
                            size_t depIdx = it->second;
                            if (unfinishedDepCount[depIdx] > 0)
                            {
                                unfinishedDepCount[depIdx]--;
                                if (unfinishedDepCount[depIdx] == 0 && nodes[depIdx].state == NodeState::pending)
                                {
                                    nodes[depIdx].state = NodeState::ready;
                                    self(self, depIdx);
                                }
                            }
                        }
                    }
                }
                else
                {
                    nodes[idx].state = NodeState::failed;
                    anyFailed = true;
                    remainingNodes--;

                    for (const auto& depId : nodes[idx].dependents)
                    {
                        auto it = nodeIndexMap.find(depId);
                        if (it != nodeIndexMap.end())
                        {
                            markBlocked(markBlocked, it->second);
                        }
                    }
                }

                cv.notify_all();
            });
        };

        {
            std::lock_guard<std::mutex> lock(mtx);
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                if (unfinishedDepCount[i] == 0)
                {
                    nodes[i].state = NodeState::ready;
                    dispatchNode(dispatchNode, i);
                }
            }
        }

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() {
                return remainingNodes == 0;
            });
        }

        return !anyFailed;
    }
}
