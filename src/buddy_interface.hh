#include "buddy_allocator.hh"

namespace KHBuddy{
    inline BuddyAllocator buddyAllocator;

    inline void* get(size_t size){
        return buddyAllocator.get(size);
    }

    inline void free(void* ptr){
        buddyAllocator.free(ptr);
    }
}