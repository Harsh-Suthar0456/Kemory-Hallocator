#pragma once

#include <cstddef>
#include <array>
#include <bitset>

#define BUDDY_MAX_SIZE 1024*16
#define BUDDY_MIN_SIZE 32
#define BUDDY_MAX_ORDER 14

constexpr int BLOCK_TABLE_SIZE = BUDDY_MAX_SIZE / BUDDY_MIN_SIZE;

class BuddyAllocator{
    private:

    void* START_ADDR;

    std::array<int, BLOCK_TABLE_SIZE> blockOrderTable;
    std::bitset<BLOCK_TABLE_SIZE> blockAllocationMap; 

    void* getFromOS(size_t size);
    void addBack(void* ptr, size_t size);
    unsigned int getOrderFromSize(size_t size);
    size_t getSizeFromOrder(unsigned int order);
    size_t getSize(void* ptr);
    void* getBuddy(void* ptr);
    void* search(void* ptr);
    void* getBuddy(void* ptr, size_t size);
    void* recursiveGet(void* currentPtr, unsigned int currentOrder, unsigned int targetOrder);
    void tryMerge(void* ptr, size_t size);

  public:
    BuddyAllocator();
    void* get(size_t size);
    void free(void* ptr);

};