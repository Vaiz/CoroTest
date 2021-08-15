#include "remote_server/cfakeserver.h"
#include "task/taskprocessor.h"
#include "task/coroutinetask.h"


class Task1 final : public CoroutineTask
{
public:
    Task1(IRemoteServerSharedPtr ptrServer, TaskProcessor& proc)
        : CoroutineTask(proc)
        , m_ptrServer(ptrServer) {}

    void Process()
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
            spdlog::info("{}: Task1: push request3", GetTaskId());
            auto response = CompleteRequest(m_ptrServer, "Request3");
            spdlog::info("{}: Task1: response 3: {}", GetTaskId(), response);
        }

        spdlog::info("{}: Task1 complete", GetTaskId());
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

        for (size_t i = 0; i < 1; ++i) {
            proc.PushTask(std::make_unique<Task1>(ptrFakeServer, proc));
        }

        std::this_thread::sleep_for(std::chrono::seconds(20));
    }


    ptrFakeServer->Shutdown();
    //sample::main2();

    return 0;
}
