#pragma once

using CoroutineTaskPtr = unique_ptr<class CoroutineTask>;

class TaskProcessor
{
    static constexpr size_t ThreadCount = 4;

    class TaskCtx
    {
    public:
        TaskCtx(CoroutineTaskPtr ptrTask);
        bool RunTask();
        TaskId GetTaskId() const;
    private:
        CoroutineTaskPtr m_ptrTask;
        Coroutine::pull_type m_yield;
    };

public:
    TaskProcessor();
    ~TaskProcessor();

    TaskId GetNextTaskId();
    void PushTask(CoroutineTaskPtr ptrTask);
    void WakeUpTask(TaskId id);

private:
    void ProcessThread();

private:
    atomic< TaskId > m_nextTaskId {0};
    sync_queue< unique_ptr< TaskCtx > > m_readyTasks;
    std::vector< std::thread > m_threads;

    std::mutex m_mutex;
    unordered_map< TaskId, unique_ptr< TaskCtx >  > m_suspendedTasks;
};
