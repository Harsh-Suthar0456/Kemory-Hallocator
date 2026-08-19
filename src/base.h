#pragma once 

#include <cstddef>
#include <cstdint>
#include <limits>

constexpr size_t PAGE_SIZE = 4096;

enum class AllocatorType : uint8_t {
    SegregatedList1, 
    SegregatedList2, 
    SegregatedList4, 
    SegregatedList8, 
    SegregatedList16,
    Invalid = std::numeric_limits<uint8_t>::max()
};

#pragma pack(push, 1)

class AllocatorBlock { 
    AllocatorType type;
  public: 
    void* get(size_t size);
    void free(void* ptr);  
};

#pragma pack(pop)