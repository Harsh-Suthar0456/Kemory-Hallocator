#include <cstddef>
#include <iostream>
#include <unistd.h>
#include <algorithm>

#include "buddy_allocator.hh"

BuddyAllocator::BuddyAllocator() {
    START_ADDR = getFromOS(BUDDY_MAX_SIZE);
    if(START_ADDR == nullptr){
        std::cerr << "Couldn't get memory from your damn OS bruh" << std::endl;
        exit(1);
    }
    freeBlocks.resize(BUDDY_MAX_ORDER + 1);
    freeBlocks[BUDDY_MAX_ORDER].push_back(START_ADDR);
    blockOrderMap[START_ADDR] = BUDDY_MAX_ORDER;
}

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

    if(currentOrder > BUDDY_MAX_ORDER){
        std::cerr << "Exceeded maximum order OR couldn't find a free block" << std::endl;
        return nullptr;
    }
    if(currentOrder < targetOrder){
        return recursiveGet(currentPtr, currentOrder+1, targetOrder);
    }
    if(freeBlocks[currentOrder].empty()){
        return recursiveGet(currentPtr, currentOrder+1, targetOrder);
    }

    void* ptr = freeBlocks[currentOrder].back();
    freeBlocks[currentOrder].pop_back();

    while(currentOrder > targetOrder){
        currentOrder--;
        size_t blockSize = getSizeFromOrder(currentOrder);
        void* buddyPtr = getBuddy(ptr, blockSize);
        freeBlocks[currentOrder].push_back(buddyPtr);
    }

    return ptr;
    
}

void* BuddyAllocator::get(size_t size){

    if(size == 0 || size > BUDDY_MAX_SIZE){
        std::cerr << "Invalid size requested" << std::endl;
        return nullptr;
    }

    unsigned int order = getOrderFromSize(size);

    void* newBlock = recursiveGet(START_ADDR, 0, order);
    if(newBlock == nullptr){
        std::cerr << "Failed to allocate memory" << std::endl;
        return nullptr;
    }

    blockOrderMap[newBlock] = order;

    return newBlock;
}



void BuddyAllocator::tryMerge(void* ptr, size_t size){
    void* buddyPtr = getBuddy(ptr, size);
    auto it = std::find(freeBlocks[getOrderFromSize(size)].begin(), freeBlocks[getOrderFromSize(size)].end(), buddyPtr);
    
    if(it != freeBlocks[getOrderFromSize(size)].end()){
        freeBlocks[getOrderFromSize(size)].erase(it);
        void* mergedPtr = (ptr < buddyPtr) ? ptr : buddyPtr;
        blockOrderMap.erase(ptr);
        blockOrderMap.erase(buddyPtr);
        blockOrderMap[mergedPtr] = getOrderFromSize(size * 2);
        tryMerge(mergedPtr, size * 2);
    } 
    else {
        freeBlocks[getOrderFromSize(size)].push_back(ptr);
    }
}

void BuddyAllocator::addBack(void* ptr, size_t size){
    blockOrderMap.erase(ptr);
    tryMerge(ptr, size);
}

void BuddyAllocator::free(void* ptr){
    if(ptr == nullptr){
        std::cerr << "You sent a nullptr Bitch" << std::endl;
        return;
    }

    size_t size = getSize(ptr);
    addBack(ptr, size);
}