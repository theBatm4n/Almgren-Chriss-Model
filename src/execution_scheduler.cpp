#include "execution_scheduler.hpp"
#include "iostream"

execution_scheduler::~execution_scheduler(){
    stop();
}


void execution_scheduler::scheduleAt(const TimePoint& time, Task task)
{
    //std::unique_lock<std::mutex> lock(mutex_);
    //this->task_.push(time, std::move(task));
}