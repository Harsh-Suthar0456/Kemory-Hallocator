#include "buddy_allocator.hh"

namespace KHBuddy{
    static BuddyAllocator buddyAllocator;

    void* get(size_t size){
        return buddyAllocator.get(size);
    }

    void free(void* ptr){
        buddyAllocator.free(ptr);
    }
}