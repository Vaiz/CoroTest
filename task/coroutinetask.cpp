#include "coroutinetask.h"
#include "taskprocessor.h"

CoroutineTask::CoroutineTask(TaskProcessor &proc)
    : m_proc(proc)
    , m_taskId(m_proc.GetNextTaskId())
{
}

TaskId CoroutineTask::GetTaskId() const
{
    return m_taskId;
}
void CoroutineTask::ProcessImpl(Coroutine::push_type& yield)
{
    pYield = &yield;
    Process();
}

Response CoroutineTask::CompleteRequest(IRemoteServerSharedPtr ptrRemoteServer, Request request)
{
    std::optional< Response > outResponse;

    auto callback = [this, &outResponse](Response response) {
        spdlog::info("{}: CoroutineTask: Response received", m_taskId);
        outResponse.emplace(std::move(response));
        m_proc.WakeUpTask(m_taskId);
    };

    ptrRemoteServer->SendRequest(std::move(request), std::move(callback));
    spdlog::info("{}: CoroutineTask: yield enter", m_taskId);
    (*pYield)();
    spdlog::info("{}: CoroutineTask: yield exit", m_taskId);

    return std::move(outResponse.value());
}
