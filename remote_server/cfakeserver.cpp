#include "cfakeserver.h"

FakeServer::FakeServer()
{
    for (size_t i = 0; i < ThreadCount; ++i)
        m_workers.emplace_back([this]{ ProcessingThread(); });
}

void FakeServer::SendRequest(Request request, Callback callback)
{
    spdlog::info("Request has been queued. [{}]", request);
    m_inQueue.push({std::move(request), std::move(callback)});
}

void FakeServer::Shutdown()
{
    spdlog::info("FakeServer: shut down request received");
    m_inQueue.close();

    for (auto& t: m_workers)
        if (t.joinable())
            t.join();

    spdlog::info("FakeServer: server has been shutted down");
}

void FakeServer::ProcessingThread()
{
    spdlog::info("FakeServer::ProcessingThread: thread started. Id: {}", std::this_thread::get_id());

    try {
        ProcessingThreadImpl();
    }  catch (const std::exception& e) {
        spdlog::error("FakeServer::ProcessingThread finished with error. Id: {}. Error: {}", std::this_thread::get_id(), e.what());
    }

    spdlog::info("FakeServer::ProcessingThread: thread finished. Id: {}", std::this_thread::get_id());
}

void FakeServer::ProcessingThreadImpl()
{
    using boost::concurrent::queue_op_status;

    std::uniform_int_distribution timeoutDistr(MinRequestProcessingTimeSec, MaxRequestProcessingTimeSec);
    std::random_device randDev;

    while(true) {
        RequestQueue::value_type item;
        const queue_op_status status = m_inQueue.wait_pull(item);

        switch (status) {
        case boost::concurrent::queue_op_status::success:
            break;
        case boost::concurrent::queue_op_status::empty:
        case boost::concurrent::queue_op_status::full:
        case boost::concurrent::queue_op_status::timeout:
        case boost::concurrent::queue_op_status::busy:
        case boost::concurrent::queue_op_status::not_ready:
            spdlog::warn("FakeServer::ProcessingThread: unexpected queue status {}", status);
            continue;
        case boost::concurrent::queue_op_status::closed:
            spdlog::info("FakeServer::ProcessingThread: queue has been stoped");
            return;
        }

        spdlog::info("FakeServer::ProcessingThread: processing request [{}]", item.first);
        std::this_thread::sleep_for(std::chrono::seconds(timeoutDistr(randDev)));
        item.second(item.first + ": Ok");
    }
}

