#pragma once

#include<iostream>
#include <unordered_map>
#include<vector>

#include "base.h"

#define BUDDY_MAX_SIZE 1024*16

/*
BuddyAllocator
- Buddy shall be calculated by taking XOR with the Size
- We could store the order of each block in a separate data structure, like a static array or a hash map, to keep track of the size of each allocated block. This way, when we need to free a block, we can look up its size and calculate its buddy accordingly. 
*/

class BuddyAllocator: public Allocator{
    private:

    void* START_ADDR;

    std::unordered_map<void*, int> blockOrderMap;
    std::vector<std::vector<void*>> freeBlocks;

    void* getFromOS(size_t size);
    void addBack(void* ptr, size_t size);
    unsigned int getOrderFromSize(size_t size);
    size_t getSizeFromOrder(unsigned int order);
    size_t getSize(void* ptr);
    void* getBuddy(void* ptr);
    void* getBuddy(void* ptr, size_t size);
    void* recursiveGet(void* currentPtr, unsigned int currentOrder, unsigned int targetOrder);
                                           
  public:
    virtual void* get(size_t size) override;
    virtual void free(void* ptr) override;

};