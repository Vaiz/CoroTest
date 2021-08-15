#pragma once
#include "taskprocessor.h"

class CoroTaskPromise final : boost::noncopyable {
public:
    CoroTaskPromise(TaskProcessor& proc, TaskId taskId, Coroutine::push_type& yield);

    void SetValue(Response response);
    Response PopValue();

private:
    TaskProcessor& m_proc;
    PromiseId m_id;
    TaskId m_taskId;
    std::optional<Response> m_response;
    Coroutine::push_type& m_yield;
};

using CoroTaskPromisePtr = unique_ptr<CoroTaskPromise>;
