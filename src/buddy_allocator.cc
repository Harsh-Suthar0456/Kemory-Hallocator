#include "buddy_allocator.hh"
#include <cstddef>
#include <unistd.h>

void* BuddyAllocator::getBuddy(void* ptr, size_t size){
    void* relPtr = (void*)((size_t)ptr - (size_t)START_ADDR);
    void* buddyRelPtr = (void*)((size_t)relPtr ^ size);
    void* buddyPtr = (void*)((size_t)buddyRelPtr + (size_t)START_ADDR);
    return buddyPtr;
}

void* BuddyAllocator::getFromOS(size_t size){
    void* ptr = sbrk(size);
    if((size_t)ptr == -1){
        std::cerr << "sbrk failed" << std::endl;
        return nullptr;
    }
    return ptr;
}

size_t BuddyAllocator::getSizeFromOrder(unsigned int order){
    return (size_t)(1 << order);
}

unsigned int BuddyAllocator::getOrderFromSize(size_t size){
    unsigned int order = 0;
    while((1<<order)<size){
        order++;
    }
    return order;
}