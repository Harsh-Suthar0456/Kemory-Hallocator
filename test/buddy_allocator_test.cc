#include <gtest/gtest.h>

// A common testing trick to access private members without modifying the header.
// If you prefer not to use this, you can add GoogleTest's FRIEND_TEST macro 
// into your buddy_allocator.hh file instead.
#define private public
#include "buddy_allocator.hh"
#undef private

class BuddyAllocatorTest : public ::testing::Test {
protected:
    BuddyAllocator allocator;

    void SetUp() override {
        // Initialize mock state before each test
        // We set a mock starting address so we can predictably test the XOR buddy math
        allocator.START_ADDR = (void*)0x1000; 
    }

    void TearDown() override {
        // Clean up after each test
        allocator.blockOrderMap.clear();
        allocator.freeBlocks.clear();
    }
};

// ---------------------------------------------------------
// Internal Math & Logic Tests
// ---------------------------------------------------------

TEST_F(BuddyAllocatorTest, OrderFromSizeCalculation) {
    EXPECT_EQ(allocator.getOrderFromSize(0), 0);
    
    // Note: getOrderFromSize(1) will cause size-1 = 0. 
    // __builtin_clzll(0) is mathematically undefined in GCC/Clang and usually returns 64 or 63.
    // Consider updating your getOrderFromSize method to handle size == 1 explicitly!
    // EXPECT_EQ(allocator.getOrderFromSize(1), 0); 
    
    EXPECT_EQ(allocator.getOrderFromSize(2), 1);
    EXPECT_EQ(allocator.getOrderFromSize(3), 2);
    EXPECT_EQ(allocator.getOrderFromSize(4), 2);
    EXPECT_EQ(allocator.getOrderFromSize(5), 3);
    EXPECT_EQ(allocator.getOrderFromSize(8), 3);
    EXPECT_EQ(allocator.getOrderFromSize(9), 4);
    EXPECT_EQ(allocator.getOrderFromSize(1024), 10);
}

TEST_F(BuddyAllocatorTest, SizeFromOrderCalculation) {
    EXPECT_EQ(allocator.getSizeFromOrder(0), 1);
    EXPECT_EQ(allocator.getSizeFromOrder(1), 2);
    EXPECT_EQ(allocator.getSizeFromOrder(2), 4);
    EXPECT_EQ(allocator.getSizeFromOrder(3), 8);
    EXPECT_EQ(allocator.getSizeFromOrder(10), 1024);
}

TEST_F(BuddyAllocatorTest, BuddyAddressCalculation) {
    // Assuming START_ADDR is 0x1000 (set in SetUp)
    // If we have a block of size 16 (0x10) at offset 0x10 (absolute 0x1010)
    // Its buddy should be at offset 0x00 (absolute 0x1000)
    
    void* ptr = (void*)0x1010;
    
    // We must populate blockOrderMap so getSize() works inside getBuddy()
    allocator.blockOrderMap[ptr] = 4; // order 4 -> size 16
    
    void* buddy = allocator.getBuddy(ptr);
    EXPECT_EQ(buddy, (void*)0x1000);
    
    // Reverse test: the buddy of 0x1000 (size 16) should be 0x1010
    allocator.blockOrderMap[(void*)0x1000] = 4;
    EXPECT_EQ(allocator.getBuddy((void*)0x1000), (void*)0x1010);
}

TEST_F(BuddyAllocatorTest, GetSizeFromMap) {
    void* mockPtr = (void*)0x2000;
    allocator.blockOrderMap[mockPtr] = 5; // Order 5 -> Size 32
    
    EXPECT_EQ(allocator.getSize(mockPtr), 32);
    
    // Unmapped pointer should return 0 based on your implementation
    void* unmappedPtr = (void*)0x3000;
    EXPECT_EQ(allocator.getSize(unmappedPtr), 0);
}

// ---------------------------------------------------------
// Public API Tests (Uncomment when implemented)
// ---------------------------------------------------------

/*
TEST_F(BuddyAllocatorTest, BasicAllocation) {
    // Tests that get() successfully allocates memory
    void* ptr1 = allocator.get(16);
    ASSERT_NE(ptr1, nullptr);
    
    // Size should be mapped and rounded up to the nearest power of 2
    EXPECT_EQ(allocator.getSize(ptr1), 16);
}

TEST_F(BuddyAllocatorTest, AllocationRounding) {
    // Asking for 17 bytes should allocate order 5 (32 bytes)
    void* ptr = allocator.get(17);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(allocator.getSize(ptr), 32);
}

TEST_F(BuddyAllocatorTest, FreeAndReuse) {
    void* ptr1 = allocator.get(32);
    ASSERT_NE(ptr1, nullptr);
    
    allocator.free(ptr1);
    
    // Assuming standard buddy allocator behavior, a new allocation 
    // of the same size should reuse the recently freed block.
    void* ptr2 = allocator.get(32);
    EXPECT_EQ(ptr1, ptr2); 
}
*/