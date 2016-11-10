#pragma once
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>
#include "AzureModels\ModelManager.h"

class TaskQueue
{
public:
    typedef std::packaged_task<std::string(ModelManager*)> Task;

    TaskQueue();

    std::future<std::string> Enqueue(Task task);
    Task Dequeue();

    void DisableEnqueue();
    void EnableEnqueue();

    // Return true if the queue accepts new items, or it has some items already queued.
    bool IsActive();

private:
    std::queue<Task> _queue;

    std::mutex _mutex;
    std::condition_variable _cv;

    // If _allowEnqueue to false, attempting to Enqueue an item will throw an exception.
    // This can be useful when we are shutting down and we need to stop accepting new requests
    // and empty out the current queue.
    bool _allowEnqueue;
};