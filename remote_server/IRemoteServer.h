#pragma once

struct IRemoteServer
{
    virtual ~IRemoteServer() = default;
    virtual void SendRequest(Request request, Callback callback) = 0;
    virtual void Shutdown() = 0;
};

using IRemoteServerSharedPtr = std::shared_ptr< IRemoteServer >;
