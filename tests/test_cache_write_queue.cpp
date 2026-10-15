// SPDX-License-Identifier: GPL-2.0-or-later
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <future>
#include <thread>
#include <vector>
#include "video_core/cache_storage_worker.h"

int main() {
    constexpr int Producers = 8, PerProducer = 1000;
    Storage::CacheWriteQueue queue;
    std::array<int, Producers> last;
    last.fill(-1);
    std::array<std::atomic<int>, Producers * PerProducer> seen{};
    std::promise<void> start;
    auto gate = start.get_future();
    std::jthread consumer([&](std::stop_token stop) {
        gate.wait();
        std::packaged_task<void()> job;
        while (queue.WaitPop(job, stop)) job();
    });
    std::vector<std::thread> producers;
    for (int p = 0; p < Producers; ++p) {
        producers.emplace_back([&, p] {
            for (int i = 0; i < PerProducer; ++i) {
                queue.Push(std::packaged_task<void()>([&, p, i] {
                    assert(last[p] + 1 == i);
                    last[p] = i;
                    assert(seen[p * PerProducer + i].fetch_add(1) == 0);
                }));
            }
        });
    }
    for (auto& producer : producers) producer.join();
    consumer.request_stop();
    start.set_value();
    consumer.join();
    for (const auto& count : seen) assert(count.load() == 1);

    // Run writers and the consumer at the same time: the old counter could lose updates.
    Storage::CacheWriteQueue concurrent;
    for (auto& count : seen) count.store(0);
    std::jthread live_consumer([&](std::stop_token stop) {
        std::packaged_task<void()> job;
        while (concurrent.WaitPop(job, stop)) job();
    });
    producers.clear();
    for (int p = 0; p < Producers; ++p) {
        producers.emplace_back([&, p] {
            for (int i = 0; i < PerProducer; ++i) {
                concurrent.Push(std::packaged_task<void()>([&, p, i] {
                    assert(seen[p * PerProducer + i].fetch_add(1) == 0);
                }));
            }
        });
    }
    for (auto& producer : producers) producer.join();
    live_consumer.request_stop();
    live_consumer.join();
    for (const auto& count : seen) assert(count.load() == 1);

    Storage::CacheWriteQueue idle;
    std::promise<void> stopped;
    auto done = stopped.get_future();
    std::jthread idle_consumer([&](std::stop_token stop) {
        std::packaged_task<void()> job;
        assert(!idle.WaitPop(job, stop));
        stopped.set_value();
    });
    idle_consumer.request_stop();
    assert(done.wait_for(std::chrono::seconds(2)) == std::future_status::ready);
    idle_consumer.join();
    std::puts("Cache write queue: PASS (16000 writes, concurrent consume, order, drain, idle wake)");
}
