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
    typedef std::packaged_task<std::wstring()> Task;

    TaskQueue();

    std::future<std::wstring> Enqueue(Task task);
    Task Dequeue();

private:
    std::queue<Task> _queue;

    std::mutex _mutex;
    std::condition_variable _cv;
};