#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <unordered_set>
#include <vector>

#include "base.h"
#include "segregated.h"

struct DummyBaseAllocator : Allocator {
    void* get(size_t) override {
        return nullptr;
    }

    void free(void*) override {}
};

using Alloc = SegregatedListAllocator<8, DummyBaseAllocator>;

TEST_CASE("SegregatedListAllocator allocates blocks", "[allocator]") {
    Alloc allocator;

    std::vector<void*> ptrs;

    constexpr size_t N = 100;

    for (size_t i = 0; i < N; ++i) {
        void* ptr = allocator.get(8);

        REQUIRE(ptr != nullptr);
        ptrs.push_back(ptr);
    }

    // Every allocation should be distinct.
    std::unordered_set<void*> unique(ptrs.begin(), ptrs.end());

    REQUIRE(unique.size() == N);
}

TEST_CASE("SegregatedListAllocator rejects wrong sizes", "[allocator]") {
    Alloc allocator;

    REQUIRE(allocator.get(1) == nullptr);
    REQUIRE(allocator.get(4) == nullptr);
    REQUIRE(allocator.get(16) == nullptr);
    REQUIRE(allocator.get(0) == nullptr);
}

TEST_CASE("SegregatedListAllocator reuses freed blocks", "[allocator]") {
    Alloc allocator;

    void* p1 = allocator.get(8);
    void* p2 = allocator.get(8);
    void* p3 = allocator.get(8);

    REQUIRE(p1 != nullptr);
    REQUIRE(p2 != nullptr);
    REQUIRE(p3 != nullptr);

    allocator.free(p2);

    void* p4 = allocator.get(8);

    REQUIRE(p4 == p2);
}

TEST_CASE("SegregatedListAllocator can reuse multiple freed blocks", "[allocator]") {
    Alloc allocator;

    std::vector<void*> ptrs;

    constexpr size_t N = 20;

    for (size_t i = 0; i < N; ++i) {
        ptrs.push_back(allocator.get(8));
    }

    // Free a few blocks.
    allocator.free(ptrs[3]);
    allocator.free(ptrs[7]);
    allocator.free(ptrs[15]);

    // The allocator is LIFO for its free list, so these should
    // come back in reverse order.
    REQUIRE(allocator.get(8) == ptrs[15]);
    REQUIRE(allocator.get(8) == ptrs[7]);
    REQUIRE(allocator.get(8) == ptrs[3]);
}