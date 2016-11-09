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

void TaskQueue::Enqueue(packaged_task<string(void)>& task)
{
    TRACE(L"TaskQueue::Enqueue()");
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

    _queue.push(move(task));

    l.unlock();
    _cv.notify_one();
}

std::packaged_task<std::string(void)> TaskQueue::Dequeue()
{
    TRACE(L"TaskQueue::Dequeue()");

    unique_lock<mutex> l(_mutex);
    _cv.wait(l, [&] { return !_queue.empty(); });

    packaged_task<string(void)> taskItem = move(_queue.front());
    _queue.pop();

    return taskItem;
}
