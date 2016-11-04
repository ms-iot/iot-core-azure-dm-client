#include "stdafx.h"
#include "TaskQueue.h"
#include "Utilities\Logger.h"

using namespace std;

TaskQueue::TaskQueue() :
    _notEmpty(false),
    _allowEnqueue(true)
{
}

void TaskQueue::DisableEnqueue()
{
    unique_lock<mutex> l(_mutex);
    _allowEnqueue = false;
}

bool TaskQueue::IsActive()
{
    unique_lock<mutex> l(_mutex);
    return _notEmpty || _allowEnqueue;
}

unsigned int TaskQueue::GetJobId()
{
    static unsigned long id = 0;
    return InterlockedIncrement(&id);
}

void TaskQueue::Enqueue(std::shared_ptr<TaskItem> taskItem)
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
        throw DMException("Warning: an attempt was made to enqueue an item after shutdown started.");
    }

    _queue.push(taskItem);
    _notEmpty = true;

    l.unlock();
    _cv.notify_one();
}

std::shared_ptr<TaskItem> TaskQueue::Dequeue()
{
    TRACE(L"TaskQueue::Dequeue()");

    unique_lock<mutex> l(_mutex);
    _cv.wait(l, [&] { return _notEmpty; });

    std::shared_ptr<TaskItem> taskItem = _queue.front();
    _queue.pop();

    if (_queue.size() == 0)
    {
        _notEmpty = false;
    }

    l.unlock();
    return taskItem;
}
