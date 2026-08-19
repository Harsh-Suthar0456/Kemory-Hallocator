#include <cstddef>
#include <iostream>
#include <new>
#include <stdexcept>
#include <unistd.h>

#include "buddy_allocator.hh"

BuddyAllocator::BuddyAllocator() {
  START_ADDR = getFromOS(BUDDY_MAX_SIZE);
  if (START_ADDR == nullptr) {
    std::cerr << "Couldn't get memory from your damn OS bruh" << std::endl;
    exit(1);
  }
  blockOrderTable.fill(0);
  blockAllocationMap.reset();
}

void *BuddyAllocator::getBuddy(void *ptr) {
  size_t size = getSize(ptr);
  return getBuddy(ptr, size);
}

void *BuddyAllocator::getBuddy(void *ptr, size_t size) {
  size_t relPtr = (size_t)ptr - (size_t)START_ADDR;
  size_t buddyRelPtr = relPtr ^ size;
  size_t buddyAddr = buddyRelPtr + (size_t)START_ADDR;
  return (void *)buddyAddr;
}

void *BuddyAllocator::getFromOS(size_t size) {
  void *ptr = sbrk(size);
  if ((size_t)ptr == (size_t)-1) {
    throw std::bad_alloc();
  }
  return ptr;
}

size_t BuddyAllocator::getSizeFromOrder(unsigned int order) {
  return (size_t)(1 << order);
}

unsigned int BuddyAllocator::getOrderFromSize(size_t size) {
  if (size == 0) {
    return 5;
  }
  unsigned int order = (sizeof(size_t) * 8 - __builtin_clzll(size - 1));
  return (order < 5) ? 5 : order;
}

size_t BuddyAllocator::getSize(void *ptr) {
  size_t index = ((size_t)ptr - (size_t)START_ADDR) / BUDDY_MIN_SIZE;
  if (index < BLOCK_TABLE_SIZE && blockOrderTable[index] != 0) {
    return getSizeFromOrder(blockOrderTable[index]);
  }
  return 0;
}

void *BuddyAllocator::recursiveGet(void *currentPtr, unsigned int currentOrder,
                                   unsigned int targetOrder) {
  size_t numBlocks = getSizeFromOrder(targetOrder) / BUDDY_MIN_SIZE;

  for (size_t i = 0; i < BLOCK_TABLE_SIZE; i += numBlocks) {
    bool isFree = true;
    for (size_t j = 0; j < numBlocks; j++) {
      if (blockAllocationMap.test(i + j)) {
        isFree = false;
        break;
      }
    }
    if (isFree) {
      return (void *)((size_t)START_ADDR + (i * BUDDY_MIN_SIZE));
    }
  }
  return nullptr;
}

void *BuddyAllocator::get(size_t size) {
  if (size == 0 || size > BUDDY_MAX_SIZE) {
    std::cerr << "Invalid size requested" << std::endl;
    return nullptr;
  }

  unsigned int order = getOrderFromSize(size);
  void *newBlock = recursiveGet(START_ADDR, 0, order);
  if (newBlock == nullptr) {
    std::cerr << "Failed to allocate memory" << std::endl;
    return nullptr;
  }

  size_t index = ((size_t)newBlock - (size_t)START_ADDR) / BUDDY_MIN_SIZE;
  size_t numBlocks = getSizeFromOrder(order) / BUDDY_MIN_SIZE;

  for (size_t j = 0; j < numBlocks; j++) {
    blockAllocationMap.set(index + j);
  }
  blockOrderTable[index] = order;

  return newBlock;
}

void BuddyAllocator::tryMerge(void *ptr, size_t size) {
  if (size >= BUDDY_MAX_SIZE) {
    return;
  }

  void *buddyPtr = getBuddy(ptr, size);

  if (buddyPtr < START_ADDR ||
      (size_t)buddyPtr >= (size_t)START_ADDR + BUDDY_MAX_SIZE) {
    return;
  }

  size_t buddyIndex = ((size_t)buddyPtr - (size_t)START_ADDR) / BUDDY_MIN_SIZE;
  size_t numBlocks = size / BUDDY_MIN_SIZE;

  bool isBuddyFree = true;
  for (size_t j = 0; j < numBlocks; j++) {
    if (blockAllocationMap.test(buddyIndex + j)) {
      isBuddyFree = false;
      break;
    }
  }

  if (isBuddyFree) {
    void *mergedPtr = (ptr < buddyPtr) ? ptr : buddyPtr;
    size_t ptrIndex = ((size_t)ptr - (size_t)START_ADDR) / BUDDY_MIN_SIZE;
    size_t mergedIndex =
        ((size_t)mergedPtr - (size_t)START_ADDR) / BUDDY_MIN_SIZE;

    blockOrderTable[ptrIndex] = 0;
    blockOrderTable[buddyIndex] = 0;
    blockOrderTable[mergedIndex] = getOrderFromSize(size * 2);

    tryMerge(mergedPtr, size * 2);
  }
}

void BuddyAllocator::addBack(void *ptr, size_t size) {
  size_t index = ((size_t)ptr - (size_t)START_ADDR) / BUDDY_MIN_SIZE;
  size_t numBlocks = size / BUDDY_MIN_SIZE;

  for (size_t j = 0; j < numBlocks; j++) {
    blockAllocationMap.reset(index + j);
  }
  blockOrderTable[index] = 0;

  tryMerge(ptr, size);
}

void *BuddyAllocator::search(void *ptr) {
  size_t index = ((size_t)ptr - (size_t)START_ADDR) / BUDDY_MIN_SIZE;

  if (index >= BLOCK_TABLE_SIZE) {
    return nullptr;
  }

  for (size_t i = index + 1; i > 0; --i) {
    size_t candidateIdx = i - 1;

    if (blockAllocationMap.test(candidateIdx) &&
        blockOrderTable[candidateIdx] != 0) {

      void *blockPtr =
          (void *)((size_t)START_ADDR + candidateIdx * BUDDY_MIN_SIZE);

      size_t blockSize = getSizeFromOrder(blockOrderTable[candidateIdx]);

      if ((size_t)ptr >= (size_t)blockPtr &&
          (size_t)ptr < (size_t)blockPtr + blockSize) {
        return blockPtr;
      }
    }
  }

  return nullptr;
}

void BuddyAllocator::free(void *ptr) {
  if (ptr == nullptr) {
    throw std::invalid_argument("Not my pointer bruh");
  }

  size_t addr = (size_t)ptr;
  size_t startAddr = (size_t)START_ADDR;

  if (addr < startAddr || addr >= startAddr + BUDDY_MAX_SIZE) {
    throw std::invalid_argument("Not my pointer bruh");
  }

  ptr = search(ptr);

  if (ptr == nullptr) {
    throw std::invalid_argument("Not my pointer bruh");
  }

  size_t size = getSize(ptr);

  if (size == 0) {
    throw std::invalid_argument("Not my pointer bruh");
  }

  addBack(ptr, size);
}