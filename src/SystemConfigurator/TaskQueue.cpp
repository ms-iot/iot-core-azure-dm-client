#include "stdafx.h"
#include "TaskQueue.h"
#include "../SharedUtilities/Logger.h"
#include "../SharedUtilities/DMException.h"

using namespace std;

TaskQueue::TaskQueue()
{
}

future<wstring> TaskQueue::Enqueue(Task task)
{
    TRACE(__FUNCTION__);

    future<wstring> response = task.get_future();

    unique_lock<mutex> l(_mutex);
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
