#pragma once
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

class TaskQueue
{
public:
    TaskQueue();

    void Enqueue(std::packaged_task<std::string(void)>& task);
    std::packaged_task<std::string(void)> Dequeue();

    void DisableEnqueue();
    void EnableEnqueue();

    // Return true if the queue accepts new items, or it has some items already queued.
    bool IsActive();

private:
    // TaskItems can be shared since in the method call case Enqueue()
    // will hold on to the TaskItem while it has been dequeued for processing.
    std::queue<std::packaged_task<std::string(void)>> _queue;

    std::mutex _mutex;
    std::condition_variable _cv;

    bool _allowEnqueue;
};