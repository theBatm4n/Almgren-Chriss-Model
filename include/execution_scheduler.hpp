#ifndef EXECUTION_SCHEDULER_HPP
#define EXECUTION_SCHEDULER_HPP

#include <functional>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <utility>

class execution_scheduler
{
public:
    using Task = std::function<void()>;
    using TimePoint = std::chrono::steady_clock::time_point;

    // represents a scheduled task with execution time and the task itself
    struct ScheduledTask 
    {
        TimePoint executionTime;
        Task Task;

        // Comparator for priority queue (earlier time has higher priority)
        bool operator<(const ScheduledTask& other) const {
            return executionTime > other.executionTime; // Min-heap behaviour
        }
    };

    execution_scheduler() = default;
    void scheduleAt(const TimePoint& time, Task task);
    void scheduleAfter(const std::chrono::milliseconds& delay, Task task);
    void schedulerEvery(const std::chrono::milliseconds& interval, Task task);

private:
    std::priority_queue<ScheduledTask> task_;
};

#endif