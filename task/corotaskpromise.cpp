#include "corotaskpromise.h"

CoroTaskPromise::CoroTaskPromise(TaskProcessor& proc, TaskId taskId, Coroutine::push_type& yield)
    : m_proc { proc }
    , m_id { m_proc.GetNextPromiseId() }
    , m_taskId { taskId }
    , m_yield { yield }
{
}

void CoroTaskPromise::SetValue(Response response)
{
    m_response.emplace(std::move(response));
    m_proc.CompletePromise(m_id);
}

Response CoroTaskPromise::PopValue()
{
    if (m_proc.AddPromise(m_id, m_taskId))
        m_yield();
    return std::move(m_response.value());
}
