#pragma once
#include "../remote_server/IRemoteServer.h"
#include "corotaskpromise.h"

class TaskProcessor;

class CoroutineTask : boost::noncopyable {
public:
    CoroutineTask(TaskProcessor& proc);

    virtual ~CoroutineTask() = default;
    virtual void Process() = 0;


    TaskId GetTaskId() const;
    void ProcessImpl(Coroutine::push_type& yield);

protected:
    Response CompleteRequest(IRemoteServerSharedPtr ptrRemoteServer, Request request);
    CoroTaskPromisePtr PushRequest(IRemoteServerSharedPtr ptrRemoteServer, Request request);

private:
    TaskProcessor &m_proc;
    TaskId m_taskId;

    Coroutine::push_type* m_pYield { nullptr };
};

using CoroutineTaskPtr = unique_ptr< CoroutineTask >;
