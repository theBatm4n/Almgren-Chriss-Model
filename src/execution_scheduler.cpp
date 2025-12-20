#include "execution_scheduler.hpp"
#include <iostream>

execution_scheduler::execution_scheduler() = default;

execution_scheduler::~execution_scheduler(){
    stop();
}

void execution_scheduler::start(){
    if(running_.exchange(true)){
        return; // already running
    }
    workerThread_ = std::thread(&execution_scheduler::workerThread, this);
}

void execution_scheduler::stop(){
    if(running_.exchange(false)){
        return;
    }
    condition_.notify_all();
    if(workerThread_.joinable()){
        workerThread_.join();
    }
}

void execution_scheduler::scheduleAt(const TimePoint& time, Task task){
    addTask(ScheduledTask{time, std::move(task)});
}

void execution_scheduler::scheduleAfter(const std::chrono::milliseconds& delay, Task task){
    auto time = std::chrono::steady_clock::now() + delay;
    scheduleAt(time, std::move(task));
}

void execution_scheduler::scheduleEvery(const std::chrono::milliseconds& interval, Task task){
    auto now = std::chrono::steady_clock::now();
    ScheduledTask scheduledTask{now, std::move(task), interval};
    addTask(scheduledTask); 
}

void execution_scheduler::addTask(const ScheduledTask& task){
    {
        std::lock_guard lock(queueMutex_);
        tasks_.push(task);
    }
    condition_.notify_one();
}

size_t execution_scheduler::pendingTasks() const {
    std::lock_guard lock(queueMutex_);
    return tasks_.size();
}

void execution_scheduler::workerThread(){
    std::cout << "Worker thread STARTED" << std::endl;
    while(running_){
        std::unique_lock lock(queueMutex_);

        if(tasks_.empty()){
            condition_.wait(lock, [this]() {return !tasks_.empty() || !running_; }); // only wake up when tasks exit or shutdown required 
            continue;
        }

        auto nextTask = tasks_.top();
        auto now = std::chrono::steady_clock::now();

        if(nextTask.executionTime <= now){
            //Execute task
            tasks_.pop();
            lock.unlock(); // Other threads can submit tasks immediately

            try{
                nextTask.task();
            } catch (const std::exception& e){
                std::cerr <<"Task execution error: " << e.what() << std::endl;
            }

            //reschedule task if its recurring
            if (nextTask.isRecurring()){
                ScheduledTask newTask{
                    now + nextTask.interval,
                    nextTask.task,
                    nextTask.interval
                };
                addTask(newTask);
            }
        } else {
            condition_.wait_until(lock, nextTask.executionTime);
        }
    }
}