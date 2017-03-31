/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
