#include "buddy_allocator.hh"
#include <cstddef>
#include <unistd.h>

void* BuddyAllocator::getBuddy(void* ptr){
    size_t size = getSize(ptr);
    return getBuddy(ptr, size);
}

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
    if(size == 0){
        return 0;
    }
    return (size <=0) ? 0 : 
                        (sizeof(size_t) * 8 - __builtin_clzll(size - 1));
}

size_t BuddyAllocator::getSize(void* ptr){
    auto it = blockOrderMap.find(ptr);
    if(it != blockOrderMap.end()){
        unsigned int order = it->second;
        return getSizeFromOrder(order);
    }
    return 0;
}

void* BuddyAllocator::recursiveGet(void* currentPtr, unsigned int currentOrder, unsigned int targetOrder){
    
}

void* BuddyAllocator::get(size_t size){
    if(size == 0 || size > BUDDY_MAX_SIZE){
        std::cerr << "Invalid size requested" << std::endl;
        return nullptr;
    }

    unsigned int order = getOrderFromSize(size);
    size_t blockSize = getSizeFromOrder(order);

    if(!freeBlocks[order].empty()){
        void* ptr = freeBlocks[order].back();
        freeBlocks[order].pop_back();
        return ptr;
    }

    void* newBlock = getFromOS(blockSize);
    if(newBlock == nullptr){
        return nullptr;
    }

    blockOrderMap[newBlock] = order;

    return newBlock;
}