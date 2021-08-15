#pragma once

using std::atomic;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;
using boost::concurrent::sync_queue;

using u32 = std::uint32_t;
using u64 = std::uint64_t;

using TaskId = u64;
using Request = std::string;
using Response = std::string;
using Callback = std::function<void(Response)>;
using RequestQueue = sync_queue< std::pair< Request, Callback > >;
using Coroutine = boost::coroutines2::coroutine< void >;
