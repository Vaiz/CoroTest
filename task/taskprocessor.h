#pragma once

using CoroutineTaskPtr = unique_ptr<class CoroutineTask>;

class TaskProcessor final : boost::noncopyable {
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
    using TaskCtxPtr = unique_ptr<TaskCtx>;

public:
    TaskProcessor();
    ~TaskProcessor();

    TaskId GetNextTaskId();
    PromiseId GetNextPromiseId();

    void PushTask(CoroutineTaskPtr ptrTask);
    void WakeUpTask(TaskId id);

    bool AddPromise(PromiseId id, TaskId taskId);
    void CompletePromise(PromiseId id);
    void SuspendTask(TaskCtxPtr task);

private:
    void ProcessThread();
    void WakeUpTaskImpl(TaskId id);

private:
    atomic< TaskId > m_nextTaskId {0};
    atomic< PromiseId > m_nextPromiseId {0};
    sync_queue<unique_ptr<TaskCtx>> m_readyTasks;

    std::mutex m_mutex;
    unordered_map<TaskId, unique_ptr<TaskCtx>> m_suspendedTasks;
    unordered_map<PromiseId, TaskId> m_suspendedPromises;
    unordered_set<PromiseId> m_completedPromises;

    std::vector<std::thread> m_threads;
};
