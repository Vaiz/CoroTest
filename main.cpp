#include "remote_server/cfakeserver.h"
#include "task/taskprocessor.h"
#include "task/coroutinetask.h"

atomic<u64> ActiveTaskCount = 0;

class Task1 final : public CoroutineTask
{
public:
    Task1(IRemoteServerSharedPtr ptrServer, TaskProcessor& proc)
        : CoroutineTask(proc)
        , m_ptrServer(ptrServer)
    {
        ++ActiveTaskCount;
    }
    ~Task1()
    {
        --ActiveTaskCount;
    }

    void Process() override
    {
        spdlog::info("{}: Task1: started", GetTaskId());
        {
            spdlog::info("{}: Task1: push request 1", GetTaskId());
            auto response = CompleteRequest(m_ptrServer, "Request1");
            spdlog::info("{}: Task1: response 1: {}", GetTaskId(), response);
        }
        {
            spdlog::info("{}: Task1: push request 2", GetTaskId());
            auto response = CompleteRequest(m_ptrServer, "Request2");
            spdlog::info("{}: Task1: response 2: {}", GetTaskId(), response);
        }
        {
            spdlog::info("{}: Task1: push request 3", GetTaskId());
            auto response = CompleteRequest(m_ptrServer, "Request3");
            spdlog::info("{}: Task1: response 3: {}", GetTaskId(), response);
        }

        spdlog::info("{}: Task1 complete", GetTaskId());
    }
private:
    IRemoteServerSharedPtr m_ptrServer;
};

class Task2 final : public CoroutineTask {
public:
    Task2(IRemoteServerSharedPtr ptrServer, TaskProcessor& proc)
        : CoroutineTask(proc)
        , m_ptrServer(ptrServer)
    {
        ++ActiveTaskCount;
    }
    ~Task2()
    {
        --ActiveTaskCount;
    }

    void Process() override
    {
        spdlog::info("{}: Task2: started", GetTaskId());

        spdlog::info("{}: Task2: push request 1", GetTaskId());
        auto ptrPromise1 = PushRequest(m_ptrServer, "Request1");

        spdlog::info("{}: Task2: push request 2", GetTaskId());
        auto ptrPromise2 = PushRequest(m_ptrServer, "Request2");

        spdlog::info("{}: Task2: push request 3", GetTaskId());
        auto ptrPromise3 = PushRequest(m_ptrServer, "Request3");

        spdlog::info("{}: Task2: wait response 3", GetTaskId());
        spdlog::info("{}: Task2: response 3: {}", GetTaskId(), ptrPromise3->PopValue());

        spdlog::info("{}: Task2: wait response 1", GetTaskId());
        spdlog::info("{}: Task2: response 1: {}", GetTaskId(), ptrPromise1->PopValue());

        spdlog::info("{}: Task2: wait response 2", GetTaskId());
        spdlog::info("{}: Task2: response 2: {}", GetTaskId(), ptrPromise2->PopValue());

        spdlog::info("{}: Task2 complete", GetTaskId());
    }

private:
    IRemoteServerSharedPtr m_ptrServer;
};

int main()
{
    spdlog::set_pattern("[%H:%M:%S.%e] <%t> %v");
    spdlog::info("Welcome to spdlog!");

    auto ptrFakeServer = std::make_shared<FakeServer>();
    {
        TaskProcessor proc;

        for (size_t i = 0; i < 10; ++i) {
            proc.PushTask(std::make_unique<Task1>(ptrFakeServer, proc));
        }

        for (size_t i = 0; i < 10; ++i) {
            proc.PushTask(std::make_unique<Task2>(ptrFakeServer, proc));
        }

        while (ActiveTaskCount > 0)
            std::this_thread::sleep_for(std::chrono::seconds(2));
    }


    ptrFakeServer->Shutdown();

    return 0;
}
