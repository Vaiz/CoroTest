#pragma once
#include <thread>
#include "IRemoteServer.h"


class FakeServer final : public IRemoteServer
{
    static constexpr std::uint64_t MinRequestProcessingTimeSec = 1;
    static constexpr std::uint64_t MaxRequestProcessingTimeSec = 5;
    static constexpr std::size_t ThreadCount = 4;

public:
    FakeServer();
    void SendRequest(Request request, Callback callback) override;
    void Shutdown() override;

private:
    void ProcessingThread();
    void ProcessingThreadImpl();

private:
    RequestQueue m_inQueue;
    std::vector< std::thread > m_workers;
};
