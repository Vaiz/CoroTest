#include "taskprocessor.h"
#include "coroutinetask.h"



TaskProcessor::TaskCtx::TaskCtx(CoroutineTaskPtr ptrTask)
    : m_ptrTask(std::move(ptrTask))
    , m_yield([&ptrTask = m_ptrTask](Coroutine::push_type& yield) { yield(); ptrTask->ProcessImpl(yield); })
{

}
bool TaskProcessor::TaskCtx::RunTask() {
    m_yield();
    return static_cast<bool>(m_yield);
}

TaskId TaskProcessor::TaskCtx::GetTaskId() const {
    return m_ptrTask->GetTaskId();
}



TaskProcessor::TaskProcessor()
{
    for (size_t i = 0; i < ThreadCount; ++i) {
        m_threads.emplace_back([this] { ProcessThread(); });
    }
}

TaskProcessor::~TaskProcessor()
{
    try {
        m_readyTasks.close();
        for (auto& t : m_threads)
            if (t.joinable())
                t.join();
    }  catch (std::exception &e) {}
}

TaskId TaskProcessor::GetNextTaskId() {
    return ++m_nextTaskId;
}

PromiseId TaskProcessor::GetNextPromiseId() {
    return ++m_nextPromiseId;
}

void TaskProcessor::PushTask(CoroutineTaskPtr ptrTask)
{
    spdlog::info("TaskProcessor: Push task {}", ptrTask->GetTaskId());
    m_readyTasks.push(std::make_unique<TaskCtx>(std::move(ptrTask)));
}

void TaskProcessor::WakeUpTask(TaskId id)
{
    std::lock_guard lock(m_mutex);
    WakeUpTaskImpl(id);
}

bool TaskProcessor::AddPromise(PromiseId id, TaskId taskId)
{
    spdlog::info("TaskProcessor: Add promise. Task {}, Promise {}", taskId, id);
    std::lock_guard lock(m_mutex);
    if (auto iter = m_completedPromises.find(id); iter != m_completedPromises.end()) {
        spdlog::info("  TaskProcessor: Promise has been already completed. Task {}, Promise {}", taskId, id);
        m_completedPromises.erase(iter);
        return false;
    } else {
        spdlog::info("  TaskProcessor: Task has been suspended. Task {}, Promise {}", taskId, id);
        m_suspendedPromises.emplace(id, taskId);
        return true;
    }
}

void TaskProcessor::CompletePromise(PromiseId id)
{
    spdlog::info("TaskProcessor: Complete promise {}", id);
    std::lock_guard lock(m_mutex);
    if (auto iter = m_suspendedPromises.find(id); iter != m_suspendedPromises.end()) {
        spdlog::info("  TaskProcessor: Move task to active tasks. Task {}, Promise {}", iter->second, id);
        WakeUpTaskImpl(iter->second);
        m_suspendedPromises.erase(iter);
    } else {
        spdlog::info("  TaskProcessor: Mark promise as completed. Promise {}", id);
        m_completedPromises.emplace(id);
    }
}

void TaskProcessor::ProcessThread()
{
    spdlog::info("TaskProcessor: worker thread started");
    try {
        while(true) {
            unique_ptr<TaskCtx> task;

            boost::concurrent::queue_op_status result = m_readyTasks.wait_pull(task);
            switch(result) {
            case boost::concurrent::queue_op_status::success:
                break;
            case boost::concurrent::queue_op_status::empty:
            case boost::concurrent::queue_op_status::full:
            case boost::concurrent::queue_op_status::busy:
            case boost::concurrent::queue_op_status::timeout:
            case boost::concurrent::queue_op_status::not_ready:
                spdlog::info("TaskProcess: Unexpected pull result {}", result);
                continue;
            case boost::concurrent::queue_op_status::closed:
                spdlog::info("TaskProcessor: worker thread finished");
                return;
            }

            TaskId id = task->GetTaskId();
            spdlog::info("TaskProcessor: Processing task {}", id);
            if (task->RunTask()) {
                spdlog::info("TaskProcessor: Task has been suspended. Task {}", id);
                m_suspendedTasks.emplace(id, std::move(task));
            } else {
                spdlog::info("TaskProcessor: Task has been completed. Task {}", id);
            }
        }
    } catch (const std::exception& e) {
        spdlog::error("TaskProcessor: thread finished with error: {}", e.what());
    }
}

void TaskProcessor::WakeUpTaskImpl(TaskId id)
{
    spdlog::info("TaskProcessor: Wake up task {}", id);
    auto iter = m_suspendedTasks.find(id);
    if (iter == m_suspendedTasks.end())
        throw std::runtime_error(fmt::format("TaskProcessor: Cannot find suspended task with id {}", id));
    auto ptrTask = std::move(iter->second);
    m_suspendedTasks.erase(iter);
    m_readyTasks.push(std::move(ptrTask));
}
