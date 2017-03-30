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
#include "stdafx.h"
#include "TaskQueue.h"
#include "Utilities\Logger.h"

using namespace std;

TaskQueue::TaskQueue() :
    _allowEnqueue(true)
{
}

void TaskQueue::DisableEnqueue()
{
    unique_lock<mutex> l(_mutex);
    _allowEnqueue = false;
}

void TaskQueue::EnableEnqueue()
{
    unique_lock<mutex> l(_mutex);
    _allowEnqueue = true;
}

bool TaskQueue::IsActive()
{
    unique_lock<mutex> l(_mutex);
    return !_queue.empty() || _allowEnqueue;
}

future<string> TaskQueue::Enqueue(Task task)
{
    TRACE(__FUNCTION__);

    unique_lock<mutex> l(_mutex);
    if (!_allowEnqueue)
    {
        // There's always a chance a request comes from a source (i.e. device twin) while the service is shutting now.
        // In such cases, it is okay to ignore it because:
        // - If it is a desired property, then it will be sent again the next time the service restarts.
        // - If it is a method, the front-end should have a mechanism to re-submit the request if the
        //   current state does not match the expected state.
        throw DMException("Warning: cannot enqueue new tasks.");
    }

    future<string> response = task.get_future();
    _queue.push(move(task));

    l.unlock();
    _cv.notify_one();

    return response;
}

TaskQueue::Task TaskQueue::Dequeue()
{
    TRACE(__FUNCTION__);

    unique_lock<mutex> l(_mutex);
    _cv.wait(l, [&] { return !_queue.empty(); });

    Task taskItem = move(_queue.front());
    _queue.pop();

    return taskItem;
}
