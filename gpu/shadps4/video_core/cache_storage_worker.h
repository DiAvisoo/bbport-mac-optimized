// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include "common/polyfill_thread.h"

namespace Storage {

// One mutex protects both the work and the wait predicate. A separate request counter
// can lose an increment when the writer decrements it, or remain nonzero with no jobs.
class CacheWriteQueue {
public:
    void Push(std::packaged_task<void()>&& job) {
        {
            std::scoped_lock lock{mutex};
            jobs.emplace(std::move(job));
        }
        ready.notify_one();
    }

    bool WaitPop(std::packaged_task<void()>& job, std::stop_token stop) {
        std::unique_lock lock{mutex};
        Common::CondvarWait(ready, lock, stop, [&] { return !jobs.empty(); });
        if (jobs.empty()) {
            return false;
        }
        // Drain queued writes on shutdown so cache files are complete before close.
        job = std::move(jobs.front());
        jobs.pop();
        return true;
    }

private:
    std::mutex mutex;
    std::condition_variable_any ready;
    std::queue<std::packaged_task<void()>> jobs;
};

} // namespace Storage
