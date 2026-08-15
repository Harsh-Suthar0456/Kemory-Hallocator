#include "buddy_allocator.hh"

void* BuddyAllocator::getBuddy(void* ptr, size_t size){
    return (ptr ^ MASK)
}