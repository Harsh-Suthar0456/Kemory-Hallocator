#include <gtest/gtest.h>

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

TEST(SegregatedListAllocatorTest, AllocatesBlocks) {
    Alloc allocator;

    std::vector<void*> ptrs;

    constexpr size_t N = 100;

    for (size_t i = 0; i < N; ++i) {
        void* ptr = allocator.get(8);

        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    std::unordered_set<void*> unique(ptrs.begin(), ptrs.end());

    EXPECT_EQ(unique.size(), N);
}

TEST(SegregatedListAllocatorTest, RejectsWrongSizes) {
    Alloc allocator;

    EXPECT_EQ(allocator.get(1), nullptr);
    EXPECT_EQ(allocator.get(4), nullptr);
    EXPECT_EQ(allocator.get(16), nullptr);
    EXPECT_EQ(allocator.get(0), nullptr);
}

TEST(SegregatedListAllocatorTest, ReusesFreedBlocks) {
    Alloc allocator;

    void* p1 = allocator.get(8);
    void* p2 = allocator.get(8);
    void* p3 = allocator.get(8);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    allocator.free(p2);

    void* p4 = allocator.get(8);

    EXPECT_EQ(p4, p2);
}

TEST(SegregatedListAllocatorTest, ReusesMultipleFreedBlocks) {
    Alloc allocator;

    std::vector<void*> ptrs;

    constexpr size_t N = 20;

    for (size_t i = 0; i < N; ++i) {
        ptrs.push_back(allocator.get(8));
    }

    allocator.free(ptrs[3]);
    allocator.free(ptrs[7]);
    allocator.free(ptrs[15]);

    // Free list is LIFO.
    EXPECT_EQ(allocator.get(8), ptrs[15]);
    EXPECT_EQ(allocator.get(8), ptrs[7]);
    EXPECT_EQ(allocator.get(8), ptrs[3]);
}