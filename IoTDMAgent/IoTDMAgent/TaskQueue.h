#pragma once
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <future>
#include <condition_variable>

enum TaskType
{
    UnknownTask,
    ExitServiceTask,
    RenewConnectionStringTask,
    RebootTask,
    RemoteWipeTask,
    ReportPropertiesTask,
    ProcessDesiredPropertiesTask
};

struct TaskItem
{
    unsigned int id;
    TaskType type;
    std::string request;
    bool waitForResponse;
    std::promise<std::string> response;

    TaskItem() :
        type(TaskType::UnknownTask),
        id(0),
        waitForResponse(false)
    {}

    virtual ~TaskItem()
    {}
};

struct DesiredPropertiesTaskItem : public TaskItem
{
    bool completeSet;

    DesiredPropertiesTaskItem() :
        completeSet(true)
    {}
};

class TaskQueue
{
public:
    TaskQueue();

    void Enqueue(std::shared_ptr<TaskItem> workItem);
    std::shared_ptr<TaskItem> Dequeue();

    void DisableEnqueue();

    // Return true if the queue accepts new items, or it has some items already queued.
    bool IsActive();

    static unsigned int GetJobId();

private:
    // TaskItems can be shared since in the method call case Enqueue()
    // will hold on to the TaskItem while it has been dequeued for processing.
    std::queue<std::shared_ptr<TaskItem>> _queue;

    std::mutex _mutex;
    std::condition_variable _cv;

    bool _notEmpty;
    bool _allowEnqueue;
};