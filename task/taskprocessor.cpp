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

void TaskProcessor::PushTask(CoroutineTaskPtr ptrTask){
    spdlog::info("TaskProcessor: Push task {}", ptrTask->GetTaskId());
    m_readyTasks.push(std::make_unique<TaskCtx>(std::move(ptrTask))); }

void TaskProcessor::WakeUpTask(TaskId id){
    spdlog::info("TaskProcessor: Wake up task {}", id);
    std::lock_guard lock(m_mutex);
    auto iter = m_suspendedTasks.find(id);
    auto ptrTask = std::move(iter->second);
    m_suspendedTasks.erase(iter);
    m_readyTasks.push(std::move(ptrTask));
}

void TaskProcessor::ProcessThread()
{
    spdlog::info("TaskProcessor: worker thread started");
    try {
        while(true) {
            unique_ptr< TaskCtx > task;

            boost::concurrent::queue_op_status result = m_readyTasks.wait_pull(task);
            switch(result) {
            case boost::concurrent::queue_op_status::success:
                break;
            case boost::concurrent::queue_op_status::empty:
            case boost::concurrent::queue_op_status::full:
            case boost::concurrent::queue_op_status::busy:
            case boost::concurrent::queue_op_status::timeout:
            case boost::concurrent::queue_op_status::not_ready:
                continue;
            case boost::concurrent::queue_op_status::closed:
                spdlog::info("TaskProcessor: worker thread finished");
                return;
            }

            TaskId id = task->GetTaskId();
            spdlog::info("TaskProcessor: Processing task {}", id);
            if (task->RunTask()) {
                spdlog::info("TaskProcessor: Task {} suspended", id);
                m_suspendedTasks.emplace(id, std::move(task));
            } else {
                spdlog::info("TaskProcessor: Task {} completed", id);
            }
        }
    } catch (const std::exception& e) {
        spdlog::info("TaskProcessor: thread finished with error: {}", e.what());
    }
}
