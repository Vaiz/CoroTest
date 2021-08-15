#pragma once
#include "../remote_server/IRemoteServer.h"

class TaskProcessor;

class CoroutineTask
{
public:
    CoroutineTask(TaskProcessor& proc);

    virtual ~CoroutineTask() = default;
    virtual void Process() = 0;


    TaskId GetTaskId() const;
    void ProcessImpl(Coroutine::push_type& yield);

protected:
    Response CompleteRequest(IRemoteServerSharedPtr ptrRemoteServer, Request request);

private:
    TaskProcessor &m_proc;
    TaskId m_taskId;

    Coroutine::push_type* pYield { nullptr };
};

using CoroutineTaskPtr = unique_ptr< CoroutineTask >;
