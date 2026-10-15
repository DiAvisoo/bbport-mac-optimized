// SPDX-License-Identifier: GPL-2.0-or-later
#include <cassert>
#include <cstdint>
#include <cstdio>
#include "bbport_region_cache.h"
int main() {
    BbMappedRegionCache cache;
    unsigned misses = 0;
    for (unsigned pass = 0; pass < 1000; ++pass) {
        for (unsigned heap = 0; heap < 16; ++heap) {
            const std::uint64_t start = (heap + 1) * 0x10000;
            if (!cache.Contains(start + 0x100, 0x200, 2)) {
                ++misses;
                cache.Remember(start, start + 0x1000, 2);
            }
        }
    }
    assert(misses == 16);
    assert(!cache.Contains(0x10000, 0x1001, 2));
    assert(!cache.Contains(0x11000, 1, 2));
    assert(!cache.Contains(0x10000, ~std::uint64_t{0}, 2));
    assert(!cache.Contains(~std::uint64_t{0}, 1, 2));
    assert(!cache.Contains(0x10100, 1, 3));
    assert(!cache.Contains(0x10100, 1, 4));
    cache.Remember(0x10000, 0x10800, 4);
    assert(cache.Contains(0x107ff, 1, 4));
    assert(!cache.Contains(0x10800, 1, 4));
    assert(!cache.Contains(0x20000, 1, 4));
    cache.Remember(0x20000, 0x21000, 5);
    assert(!cache.Contains(0x20000, 1, 5));
    std::puts("Mapped region cache: PASS (alternating heaps, generation invalidation, boundaries)");
}
