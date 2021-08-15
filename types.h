#pragma once

using boost::concurrent::sync_queue;
using std::atomic;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;

using u32 = std::uint32_t;
using u64 = std::uint64_t;

using TaskId = u64;
using PromiseId = u64;
using Request = std::string;
using Response = std::string;
using Callback = std::function<void(Response)>;
using RequestQueue = sync_queue< std::pair< Request, Callback > >;
using Coroutine = boost::coroutines2::coroutine< void >;
