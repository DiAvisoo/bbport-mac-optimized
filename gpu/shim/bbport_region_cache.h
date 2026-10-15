// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// Per-thread cache of mapping extents, invalidated by the runtime's mapping generation.
// Draw preparation alternates between several heaps; one entry made all the workers
// contend on the native process's shared mapping-table mutex for each switch.
class BbMappedRegionCache {
public:
    bool Contains(std::uint64_t address, std::uint64_t size, std::uint64_t version) {
        if ((version & 1) || version != generation || count == 0) return false;
        const auto fits = [&](const Region& r) {
            return address >= r.start && address < r.end && size <= r.end - address;
        };
        if (fits(regions[last])) return true;
        for (std::size_t i = 0; i < count; ++i) {
            if (i != last && fits(regions[i])) { last = i; return true; }
        }
        return false;
    }
    void Remember(std::uint64_t start, std::uint64_t end, std::uint64_t version) {
        if ((version & 1) || start >= end) return;
        if (generation != version) { generation = version; count = next = 0; }
        regions[next] = {start, end};
        last = next;
        next = (next + 1) % regions.size();
        if (count < regions.size()) ++count;
    }
private:
    struct Region { std::uint64_t start = 0, end = 0; };
    std::array<Region, 16> regions{};
    std::uint64_t generation = 1;
    std::size_t count = 0, next = 0, last = 0;
};
