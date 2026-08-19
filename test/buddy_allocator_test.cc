#include <gtest/gtest.h>

// Trick to expose private members for unit testing
#define private public
#include "buddy_allocator.hh"
#undef private

class BuddyAllocatorTest : public ::testing::Test {
protected:
    BuddyAllocator* allocator;

    void SetUp() override {
        allocator = new BuddyAllocator();
    }

    void TearDown() override {
        delete allocator;
    }
};

// ---------------------------------------------------------
// Internal Logic Tests
// ---------------------------------------------------------

TEST_F(BuddyAllocatorTest, OrderFromSizeCalculation) {
    EXPECT_EQ(allocator->getOrderFromSize(0), 5);
    EXPECT_EQ(allocator->getOrderFromSize(1), 5);
    EXPECT_EQ(allocator->getOrderFromSize(2), 5);
    EXPECT_EQ(allocator->getOrderFromSize(16), 5);
    EXPECT_EQ(allocator->getOrderFromSize(31), 5);
    EXPECT_EQ(allocator->getOrderFromSize(32), 5);
    EXPECT_EQ(allocator->getOrderFromSize(33), 6);
    EXPECT_EQ(allocator->getOrderFromSize(64), 6);
    EXPECT_EQ(allocator->getOrderFromSize(65), 7);
    EXPECT_EQ(allocator->getOrderFromSize(1024), 10);
    EXPECT_EQ(allocator->getOrderFromSize(1025), 11);
    EXPECT_EQ(allocator->getOrderFromSize(16384), 14);
}

TEST_F(BuddyAllocatorTest, SizeFromOrderCalculation) {
    EXPECT_EQ(allocator->getSizeFromOrder(5), 32);
    EXPECT_EQ(allocator->getSizeFromOrder(6), 64);
    EXPECT_EQ(allocator->getSizeFromOrder(10), 1024);
    EXPECT_EQ(allocator->getSizeFromOrder(14), 16384);
}

TEST_F(BuddyAllocatorTest, BuddyAddressCalculation) {
    void* base = allocator->START_ADDR;

    void* ptr = (void*)((size_t)base + 32);

    void* buddy = allocator->getBuddy(ptr, 32);
    EXPECT_EQ(buddy, base);

    EXPECT_EQ(allocator->getBuddy(base, 32), ptr);
}

// ---------------------------------------------------------
// Public API Tests
// ---------------------------------------------------------

TEST_F(BuddyAllocatorTest, BasicAllocation) {
    void* ptr1 = allocator->get(10);
    ASSERT_NE(ptr1, nullptr);

    // 10 bytes -> 32 bytes -> order 5
    EXPECT_EQ(allocator->getSize(ptr1), 32);

    size_t index = ((size_t)ptr1 - (size_t)allocator->START_ADDR)
                   / BUDDY_MIN_SIZE;

    // The allocated block occupies exactly one minimum-sized block.
    EXPECT_TRUE(allocator->blockAllocationMap.test(index));

    // Its metadata should record order 5.
    EXPECT_EQ(allocator->blockOrderTable[index], 5);

    // The rest of the arena should remain free.
    EXPECT_FALSE(allocator->blockAllocationMap.test(index + 1));
}

TEST_F(BuddyAllocatorTest, FreeAndMerge) {
    void* ptr1 = allocator->get(16);
    void* ptr2 = allocator->get(16);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // 16 bytes are rounded up to 32 bytes.
    EXPECT_EQ(allocator->getSize(ptr1), 32);
    EXPECT_EQ(allocator->getSize(ptr2), 32);

    // They should be buddies.
    EXPECT_EQ(allocator->getBuddy(ptr1, 32), ptr2);
    EXPECT_EQ(allocator->getBuddy(ptr2, 32), ptr1);

    size_t index1 = ((size_t)ptr1 - (size_t)allocator->START_ADDR)
                    / BUDDY_MIN_SIZE;
    size_t index2 = ((size_t)ptr2 - (size_t)allocator->START_ADDR)
                    / BUDDY_MIN_SIZE;

    // Both blocks are allocated.
    EXPECT_TRUE(allocator->blockAllocationMap.test(index1));
    EXPECT_TRUE(allocator->blockAllocationMap.test(index2));

    // Free the first block.
    allocator->free(ptr1);

    EXPECT_FALSE(allocator->blockAllocationMap.test(index1));
    EXPECT_TRUE(allocator->blockAllocationMap.test(index2));
    EXPECT_EQ(allocator->blockOrderTable[index1], 0);

    // Free the second block. The two 32-byte buddies should merge,
    // and then recursively merge all the way to the full arena.
    allocator->free(ptr2);

    for (size_t i = 0; i < BLOCK_TABLE_SIZE; ++i) {
        EXPECT_FALSE(allocator->blockAllocationMap.test(i));
    }

    EXPECT_EQ(allocator->blockOrderTable[0], BUDDY_MAX_ORDER);

    for (size_t i = 1; i < BLOCK_TABLE_SIZE; ++i) {
        EXPECT_EQ(allocator->blockOrderTable[i], 0);
    }
}

TEST_F(BuddyAllocatorTest, FreeInteriorPointer) {
    void* ptr = allocator->get(100);
    ASSERT_NE(ptr, nullptr);

    EXPECT_EQ(allocator->getSize(ptr), 128);

    void* interiorPtr = (void*)((size_t)ptr + 50);

    allocator->free(interiorPtr);

    EXPECT_EQ(allocator->blockOrderTable[0], BUDDY_MAX_ORDER);

    for (size_t i = 1; i < BLOCK_TABLE_SIZE; ++i) {
        EXPECT_EQ(allocator->blockOrderTable[i], 0);
    }

    for (size_t i = 0; i < BLOCK_TABLE_SIZE; ++i) {
        EXPECT_FALSE(allocator->blockAllocationMap.test(i));
    }
}

TEST_F(BuddyAllocatorTest, InvalidAllocations) {
    EXPECT_EQ(allocator->get(0), nullptr);
    EXPECT_EQ(allocator->get(BUDDY_MAX_SIZE + 1), nullptr);
}

TEST_F(BuddyAllocatorTest, InvalidFree) {
    int x;

    EXPECT_THROW(
        allocator->free(&x),
        std::invalid_argument
    );
}

TEST_F(BuddyAllocatorTest, DoubleFree) {
    void* ptr = allocator->get(32);
    ASSERT_NE(ptr, nullptr);

    allocator->free(ptr);

    EXPECT_THROW(
        allocator->free(ptr),
        std::invalid_argument
    );
}