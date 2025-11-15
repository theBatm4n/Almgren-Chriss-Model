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
#include <memory>

class execution_scheduler
{
public:
    using Task = std::function<void()>;
    using TimePoint = std::chrono::steady_clock::time_point;

    // represents a scheduled task with execution time and the task itself
    struct ScheduledTask{
        TimePoint executionTime;
        Task task;
        std::chrono::milliseconds interval{0}; // for recurring tasks 
        bool isRecurring() const {
            return interval.count() > 0; 
        }

        // (earlier time has higher priority)
        bool operator<(const ScheduledTask& other) const {
            return executionTime > other.executionTime; // Min-heap behaviour
        }
    };

    execution_scheduler();
    ~execution_scheduler();

    //Disable Copy 
    execution_scheduler(const execution_scheduler&) = delete;
    execution_scheduler& operator=(const execution_scheduler&) = delete;

    //Allow move
    execution_scheduler(execution_scheduler&&) = default;
    execution_scheduler& operator=(execution_scheduler&&) = default;

    void start();
    void stop();
    bool isRunning() const { return running_.load(); }

    void scheduleAt(const TimePoint& time, Task task);
    void scheduleAfter(const std::chrono::milliseconds& delay, Task task);
    void scheduleEvery(const std::chrono::milliseconds& interval, Task task);

    size_t pendingTasks() const;

private:
    void workerThread();
    void addTask(const ScheduledTask& task);

    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::priority_queue<ScheduledTask> tasks_;
    std::atomic<bool> running_{false};
    std::thread workerThread_;
};

#endif