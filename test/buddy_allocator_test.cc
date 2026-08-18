#include <gtest/gtest.h>

// Trick to expose private members for unit testing
#define private public
#include "buddy_allocator.hh"
#undef private

class BuddyAllocatorTest : public ::testing::Test {
protected:
    // We use a pointer so we can create a fresh allocator for each test,
    // ensuring an empty blockOrderMap and a clean freeBlocks vector.
    BuddyAllocator* allocator;

    void SetUp() override {
        // This will call your new constructor and run getFromOS(BUDDY_MAX_SIZE)
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
    EXPECT_EQ(allocator->getOrderFromSize(0), 0);
    EXPECT_EQ(allocator->getOrderFromSize(1), 0);
    EXPECT_EQ(allocator->getOrderFromSize(2), 1);
    EXPECT_EQ(allocator->getOrderFromSize(3), 2);
    EXPECT_EQ(allocator->getOrderFromSize(4), 2);
    EXPECT_EQ(allocator->getOrderFromSize(5), 3);
    EXPECT_EQ(allocator->getOrderFromSize(16), 4);
    EXPECT_EQ(allocator->getOrderFromSize(1024), 10);
    EXPECT_EQ(allocator->getOrderFromSize(1025), 11);
}

TEST_F(BuddyAllocatorTest, SizeFromOrderCalculation) {
    EXPECT_EQ(allocator->getSizeFromOrder(0), 1);
    EXPECT_EQ(allocator->getSizeFromOrder(3), 8);
    EXPECT_EQ(allocator->getSizeFromOrder(10), 1024);
    EXPECT_EQ(allocator->getSizeFromOrder(14), 16384); // BUDDY_MAX_SIZE
}

TEST_F(BuddyAllocatorTest, BuddyAddressCalculation) {
    // We dynamically use your actual START_ADDR so this test works 
    // regardless of what memory address the OS gives us via sbrk().
    void* base = allocator->START_ADDR;
    
    // Block size 16 (order 4) at offset 16 (0x10) from the base
    void* ptr = (void*)((size_t)base + 16);
    
    // We expect the buddy of the block at offset 16 to be the block at offset 0 (base)
    void* buddy = allocator->getBuddy(ptr, 16);
    EXPECT_EQ(buddy, base);
    
    // Reverse test: buddy of base (offset 0) with size 16 should be base + 16
    EXPECT_EQ(allocator->getBuddy(base, 16), ptr);
}

// ---------------------------------------------------------
// Public API Tests (Allocation & Freeing)
// ---------------------------------------------------------

TEST_F(BuddyAllocatorTest, BasicAllocation) {
    // Requesting 10 bytes -> Should allocate 16 bytes (Order 4)
    void* ptr1 = allocator->get(10);
    ASSERT_NE(ptr1, nullptr);
    
    // Check if get() successfully mapped the block order
    EXPECT_EQ(allocator->getSize(ptr1), 16);
    
    // Verify recursive splitting happened correctly:
    // Started with one block of Order 14, requested Order 4.
    // Order 14 is popped, and the buddies for 13, 12, ... 4 are added back.
    EXPECT_EQ(allocator->freeBlocks[14].size(), 0); // Max block was split
    EXPECT_EQ(allocator->freeBlocks[13].size(), 1); // Remaining half of 14
    EXPECT_EQ(allocator->freeBlocks[5].size(), 1);  // Just above target order
}

TEST_F(BuddyAllocatorTest, FreeAndMerge) {
    void* ptr1 = allocator->get(16);
    void* ptr2 = allocator->get(16);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    
    // Because they were allocated sequentially from a split block, 
    // ptr1 and ptr2 should be buddies.
    EXPECT_EQ(allocator->getBuddy(ptr1, 16), ptr2);
    
    // Free first block. No merge possible yet because its buddy (ptr2) is still allocated.
    allocator->free(ptr1);
    EXPECT_EQ(allocator->freeBlocks[4].size(), 1); // Order 4 list now holds ptr1
    
    // Free second block. This should trigger a recursive merge all the way back up!
    allocator->free(ptr2);
    
    // The entire pool should be merged back into a single Order 14 block.
    EXPECT_EQ(allocator->freeBlocks[4].size(), 0);
    EXPECT_EQ(allocator->freeBlocks[14].size(), 1);
    
    // The fully merged block should be right back at START_ADDR
    EXPECT_EQ(allocator->freeBlocks[14][0], allocator->START_ADDR);
    
    // Verify blockOrderMap was updated to track the newly merged giant block
    EXPECT_EQ(allocator->blockOrderMap[allocator->START_ADDR], 14);
}

TEST_F(BuddyAllocatorTest, InvalidAllocations) {
    // Requesting 0 bytes
    EXPECT_EQ(allocator->get(0), nullptr);
    
    // Requesting more than max size
    EXPECT_EQ(allocator->get(BUDDY_MAX_SIZE + 1), nullptr);
}