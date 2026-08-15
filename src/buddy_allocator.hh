#pragma once

#include<iostream>
#include<vector>

#include "base.h"

#define BUDDY_MAX_SIZE 1024*16

/*
BuddyAllocator
- Buddy shall be calculated by taking XOR with the Size
- To get the size of the memory when it has to be freed, we shall find the Lowest Set Bit(LSB). Given that the OS might give us a start address that is not a power of 2, we shall store a MASK to first XOR with that and convert to power of 2 format, for the start address
*/

class BuddyAllocator: public Allocator{
    private:

    static void* MASK;

    void* getFromOS(size_t size);
    void addBack(void* ptr, size_t size);
    void* getBuddy(void* ptr, size_t size);
                                           
  public:
    virtual void* get(size_t size) override;
    virtual void free(void* ptr) override;
};