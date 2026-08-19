#include "gtest/gtest.h"
#include <gtest/gtest.h>

#include <cstddef>
#include <unordered_set>
#include <vector>

#include "segregated.h"

using Alloc = SegregatedListBlock<8>;

static std::vector<std::pair<void*, size_t>> buddy_allocations;

void* buddy_get(size_t size) {
    void* ptr = malloc(size);
    buddy_allocations.emplace_back(ptr, size);
    return ptr;
}

void buddy_free(void* ptr) {
    free(ptr);
}

TEST(SegregatedListBlockTest, Layout) {
    constexpr size_t Bytes = 8;

    constexpr size_t MaxCount =
        std::max(1uz << Bytes, PAGE_SIZE / Bytes);

    constexpr size_t BlockSize =
        MaxCount * Bytes;

    constexpr size_t BufferSize =
        BlockSize - Alloc::MetadataSize - Alloc::Padding;

    constexpr size_t BufferCount =
        BufferSize / Bytes;

    EXPECT_EQ(sizeof(Alloc), BlockSize);

    EXPECT_EQ(sizeof(Alloc) % alignof(Alloc), 0);

    EXPECT_EQ(BufferSize % Bytes, 0);
    EXPECT_EQ(BufferCount * Bytes, BufferSize);

    EXPECT_GE(BufferCount, 1);
}

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

TEST(SegregatedListBlockTest, CreatesNewBlockWhenFull) {
    buddy_allocations.clear();

    Alloc allocator;

    const size_t initial_allocations = buddy_allocations.size();

    // Keep allocating until the second buddy allocation occurs.
    while (buddy_allocations.size() < initial_allocations + 2) {
        ASSERT_NE(allocator.get(8), nullptr);
    }

    ASSERT_EQ(buddy_allocations.size(), initial_allocations + 2);

    EXPECT_EQ(
        buddy_allocations[0].second,
        buddy_allocations[1].second
    );
}

TEST(SegregatedListBlockTest, AllocatesNewBlock) {
    Alloc allocator;

    constexpr size_t N =
        (std::max(1uz << 8, PAGE_SIZE / 8) * 8
         - sizeof(AllocatorBlock)
         - sizeof(uint64_t) * 2
         - sizeof(Alloc*)) / 8;

    std::vector<void*> ptrs;
    ptrs.reserve(N + 1);

    // Fill the first block completely.
    for (size_t i = 0; i < N; ++i) {
        void* ptr = allocator.get(8);

        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // This allocation must come from a new block.
    void* next = allocator.get(8);

    ASSERT_NE(next, nullptr);

    // It must not overlap with anything in the first block.
    for (void* ptr : ptrs) {
        EXPECT_NE(next, ptr);
    }

    ptrs.push_back(next);

    // Every allocation should still be unique.
    std::unordered_set<void*> unique(ptrs.begin(), ptrs.end());

    EXPECT_EQ(unique.size(), ptrs.size());
}

TEST(SegregatedListBlockTest, AllocatesAcrossMultipleBlocks) {
    Alloc allocator;

    constexpr size_t N =
        (std::max(1uz << 8, PAGE_SIZE / 8) * 8
         - sizeof(AllocatorBlock)
         - sizeof(uint64_t) * 2
         - sizeof(Alloc*)) / 8;

    constexpr size_t Blocks = 3;

    std::vector<void*> ptrs;
    ptrs.reserve(N * Blocks);

    for (size_t i = 0; i < N * Blocks; ++i) {
        void* ptr = allocator.get(8);

        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    std::unordered_set<void*> unique(ptrs.begin(), ptrs.end());

    EXPECT_EQ(unique.size(), ptrs.size());
}

TEST(SegregatedListBlockTest, FreesAcrossBlocks) {
    Alloc allocator;

    constexpr size_t N =
        (std::max(1uz << 8, PAGE_SIZE / 8) * 8
         - sizeof(AllocatorBlock)
         - sizeof(uint64_t) * 2
         - sizeof(Alloc*)) / 8;

    // Force creation of a second block.
    std::vector<void*> firstBlock;
    firstBlock.reserve(N);

    for (size_t i = 0; i < N; ++i) {
        firstBlock.push_back(allocator.get(8));
    }

    void* secondBlockPtr = allocator.get(8);

    ASSERT_NE(secondBlockPtr, nullptr);

    // Free an allocation in the first block.
    void* freed = firstBlock[N / 2];
    allocator.free(freed);

    // The free-list entry should be reused before allocating
    // another block.
    EXPECT_EQ(allocator.get(8), freed);
}
