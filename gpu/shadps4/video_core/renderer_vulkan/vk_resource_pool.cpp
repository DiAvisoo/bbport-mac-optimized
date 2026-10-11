// SPDX-FileCopyrightText: Copyright 2024 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <optional>

#include "common/assert.h"
#include "video_core/renderer_vulkan/vk_instance.h"
#include "video_core/renderer_vulkan/vk_resource_pool.h"
#include "video_core/renderer_vulkan/vk_semaphore.h"

namespace Vulkan {

ResourcePool::ResourcePool(Semaphore* work_semaphore_, std::size_t grow_step_)
    : work_semaphore{work_semaphore_}, grow_step{grow_step_} {}

std::size_t ResourcePool::CommitResource() {
    u64 gpu_tick = work_semaphore->KnownGpuTick();
    const auto search = [this, gpu_tick](std::size_t begin,
                                         std::size_t end) -> std::optional<std::size_t> {
        for (std::size_t iterator = begin; iterator < end; ++iterator) {
            if (gpu_tick >= ticks[iterator]) {
                ticks[iterator] = work_semaphore->CurrentTick();
                return iterator;
            }
        }
        return std::nullopt;
    };

    // Try to find a free resource from the hinted position to the end.
    auto found = search(hint_iterator, ticks.size());
    if (!found) {
        // Refresh semaphore to query updated results
        work_semaphore->Refresh();
        gpu_tick = work_semaphore->KnownGpuTick();
        found = search(hint_iterator, ticks.size());
    }
    if (!found) {
        // Search from beginning to the hinted position.
        found = search(0, hint_iterator);
        if (!found) {
            // Both searches failed, the pool is full; handle it.
            const std::size_t free_resource = ManageOverflow();

            ticks[free_resource] = work_semaphore->CurrentTick();
            found = free_resource;
        }
    }

    // Free iterator is hinted to the resource after the one that's been commited.
    hint_iterator = (*found + 1) % ticks.size();
    return *found;
}

std::size_t ResourcePool::ManageOverflow() {
    const std::size_t old_capacity = ticks.size();
    ticks.resize(old_capacity + grow_step);
    Allocate(old_capacity, old_capacity + grow_step);
    // bbport (diagnostics, issue #6): a pool never shrinks, and on KosmicKrisp each command
    // buffer keeps the upload memory and Metal command allocators of its last recording until it
    // is reused. Printed at 32, 64, 128, ... command buffers.
    const std::size_t capacity = ticks.size();
    if (capacity >= 32 && (capacity & (capacity - 1)) == 0) {
        std::printf("GPU: command buffer pool %p grew to %zu (GPU finished tick %llu of %llu)\n",
                    static_cast<const void*>(this), capacity,
                    static_cast<unsigned long long>(work_semaphore->KnownGpuTick()),
                    static_cast<unsigned long long>(work_semaphore->CurrentTick()));
    }
    return old_capacity;
}

constexpr std::size_t COMMAND_BUFFER_POOL_SIZE = 4;

CommandPool::CommandPool(const Instance& instance, Semaphore* work_semaphore)
    : ResourcePool{work_semaphore, COMMAND_BUFFER_POOL_SIZE}, instance{instance} {
    const vk::CommandPoolCreateInfo pool_create_info = {
        .flags = vk::CommandPoolCreateFlagBits::eTransient |
                 vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = instance.GetGraphicsQueueFamilyIndex(),
    };
    const vk::Device device = instance.GetDevice();
    auto [pool_result, pool] = device.createCommandPoolUnique(pool_create_info);
    ASSERT_MSG(pool_result == vk::Result::eSuccess, "Failed to create command pool: {}",
               vk::to_string(pool_result));
    cmd_pool = std::move(pool);
    SetObjectName(device, *cmd_pool, "CommandPool");
}

CommandPool::~CommandPool() = default;

void CommandPool::Allocate(std::size_t begin, std::size_t end) {
    cmd_buffers.resize(end);

    const vk::CommandBufferAllocateInfo buffer_alloc_info = {
        .commandPool = *cmd_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = COMMAND_BUFFER_POOL_SIZE,
    };

    const vk::Device device = instance.GetDevice();
    const auto result =
        device.allocateCommandBuffers(&buffer_alloc_info, cmd_buffers.data() + begin);
    ASSERT(result == vk::Result::eSuccess);

    for (std::size_t i = begin; i < end; ++i) {
        SetObjectName(device, cmd_buffers[i], "CommandPool: Command Buffer {}", i);
    }
}

vk::CommandBuffer CommandPool::Commit() {
    // bbport (issue #6): the pool grows to the most command buffers ever committed within the
    // GPU's last couple of ticks (a loading frame: hundreds) and never shrank. On KosmicKrisp each
    // idle command buffer keeps the memory of its last recording (Metal command allocators, upload
    // blocks): with 256-512 of them the driver held 7-15 GB more than the renderer's own. Every
    // 64 commits, idle buffers beyond a working set are freed, and their memory with them.
    // BB_CMDBUF_TRIM=0: never (as before).
    static const bool trim = [] {
        const char* env = std::getenv("BB_CMDBUF_TRIM");
        return !(env && env[0] == '0');
    }();
    if (trim && ++commits_since_trim >= 64) {
        commits_since_trim = 0;
        Trim();
    }
    const std::size_t index = CommitResource();
    return cmd_buffers[index];
}

void CommandPool::Trim() {
    // Idle command buffers kept for reuse; more are allocated (4 at a time) when needed.
    constexpr std::size_t KeepIdle = 32;
    if (ticks.size() <= KeepIdle) {
        return;
    }
    // Idle: the GPU has finished the tick it was committed in (CommitResource's own test).
    const u64 gpu_tick = work_semaphore->KnownGpuTick();
    std::vector<vk::CommandBuffer> freed;
    std::size_t kept_idle = 0, kept = 0;
    for (std::size_t i = 0; i < ticks.size(); ++i) {
        const bool idle = gpu_tick >= ticks[i];
        if (idle && kept_idle >= KeepIdle) {
            freed.push_back(cmd_buffers[i]);
            continue;
        }
        kept_idle += idle ? 1 : 0;
        ticks[kept] = ticks[i];
        cmd_buffers[kept] = cmd_buffers[i];
        ++kept;
    }
    if (freed.empty()) {
        return;
    }
    const std::size_t before = ticks.size();
    ticks.resize(kept);
    cmd_buffers.resize(kept);
    hint_iterator = 0;
    instance.GetDevice().freeCommandBuffers(*cmd_pool, freed);
    if (freed.size() >= 32) {
        std::printf("GPU: command buffer pool %p trimmed from %zu to %zu\n",
                    static_cast<const void*>(this), before, kept);
    }
}

DescriptorHeap::DescriptorHeap(const Instance& instance, Semaphore* work_semaphore_,
                               std::span<const vk::DescriptorPoolSize> pool_sizes_,
                               u32 descriptor_heap_count_)
    : device{instance.GetDevice()}, work_semaphore{work_semaphore_},
      descriptor_heap_count{descriptor_heap_count_}, pool_sizes{pool_sizes_} {
    CreateDescriptorPool();
}

DescriptorHeap::~DescriptorHeap() {
    device.destroyDescriptorPool(curr_pool);
    for (const auto [pool, tick] : pending_pools) {
        work_semaphore->Wait(tick);
        device.destroyDescriptorPool(pool);
    }
}

vk::DescriptorSet DescriptorHeap::Commit(vk::DescriptorSetLayout set_layout) {
    const u64 set_key = std::bit_cast<u64>(set_layout);
    const auto [it, _] = descriptor_sets.try_emplace(set_key);

    // Check if allocated sets exist and pick one.
    if (!it->second.empty()) {
        const auto desc_set = it->second.back();
        it.value().pop_back();
        return desc_set;
    }

    DescSetBatch desc_sets(DescriptorSetBatch);
    std::array<vk::DescriptorSetLayout, DescriptorSetBatch> layouts;
    layouts.fill(set_layout);

    vk::DescriptorSetAllocateInfo alloc_info = {
        .descriptorPool = curr_pool,
        .descriptorSetCount = DescriptorSetBatch,
        .pSetLayouts = layouts.data(),
    };

    // Attempt to allocate the descriptor set batch.
    auto result = device.allocateDescriptorSets(&alloc_info, desc_sets.data());
    if (result == vk::Result::eSuccess) {
        const auto desc_set = desc_sets.back();
        desc_sets.pop_back();
        it.value() = std::move(desc_sets);
        return desc_set;
    }

    // The pool has run out. Record current tick and place it in pending list.
    ASSERT_MSG(result == vk::Result::eErrorOutOfPoolMemory ||
                   result == vk::Result::eErrorFragmentedPool,
               "Unexpected error during descriptor set allocation: {}", vk::to_string(result));
    pending_pools.emplace_back(curr_pool, work_semaphore->CurrentTick());
    if (const auto [pool, tick] = pending_pools.front(); work_semaphore->IsFree(tick)) {
        curr_pool = pool;
        pending_pools.pop_front();

        const auto reset_result = device.resetDescriptorPool(curr_pool);
        ASSERT_MSG(reset_result == vk::Result::eSuccess,
                   "Unexpected error resetting descriptor pool: {}", vk::to_string(reset_result));
    } else {
        CreateDescriptorPool();
    }

    // Attempt to allocate again with fresh pool.
    alloc_info.descriptorPool = curr_pool;
    result = device.allocateDescriptorSets(&alloc_info, desc_sets.data());
    ASSERT_MSG(result == vk::Result::eSuccess,
               "Unexpected error during descriptor set allocation {}", vk::to_string(result));

    // We've changed pool so also reset descriptor batch cache.
    descriptor_sets.clear();
    const auto desc_set = desc_sets.back();
    desc_sets.pop_back();
    descriptor_sets[set_key] = std::move(desc_sets);
    return desc_set;
}

void DescriptorHeap::CreateDescriptorPool() {
    const vk::DescriptorPoolCreateInfo pool_info = {
        .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
        .maxSets = descriptor_heap_count,
        .poolSizeCount = static_cast<u32>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };
    auto [pool_result, pool] = device.createDescriptorPool(pool_info);
    ASSERT_MSG(pool_result == vk::Result::eSuccess, "Failed to create descriptor pool: {}",
               vk::to_string(pool_result));
    curr_pool = pool;
}

} // namespace Vulkan
