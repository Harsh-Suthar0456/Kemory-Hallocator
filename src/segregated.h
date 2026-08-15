#pragma once

#include <algorithm>
#include <cstdint>

#include "base.h"

namespace {
template <size_t Bytes> struct unsigned_integral {};

template <> struct unsigned_integral<1> {
  using type = uint8_t;
};
template <> struct unsigned_integral<2> {
  using type = uint16_t;
};
template <> struct unsigned_integral<4> {
  using type = uint32_t;
};
template <> struct unsigned_integral<8> {
  using type = uint64_t;
};

template <size_t Bytes> 
using unsigned_integral_t = unsigned_integral<Bytes>::type;
} // namespace



template <size_t Bytes, typename BaseAllocator> // Add a default base allocator
class SegregatedListAllocator : public Allocator {
    
    using int_type = unsigned_integral_t<Bytes>;

    struct Header{
        int_type head;
        int_type used;
        SegregatedListAllocator* next;
    };

    constexpr static size_t MaxCount = std::max(1uz << Bytes, PAGE_SIZE / Bytes);
    constexpr static size_t BlockSize = MaxCount * Bytes; 
    constexpr static size_t BufferSize = BlockSize - sizeof(Header);
    constexpr static size_t BufferCount = BufferSize / Bytes;

    Header header;
    int_type data[BufferCount];

  public:

    static SegregatedListAllocator* newAllocator() {
        // get memory using BaseAllocator
        return nullptr;
    }
    
    void* get(size_t size) override {
        if (size != Bytes) [[unlikely]] {
            return nullptr;
        }
        if(header.head >= BufferCount) {
            if(header.used < BufferCount) [[likely]] {
                int_type offset = header.used++;
                return &data[offset];
            } else {
                if (!header.next) {
                    header.next = newAllocator();
                } 
                return header.next->get(size);
            }
        } else {
            int_type temp = header.head;
            header.head = data[header.head];
            return &data[temp];
        }
    }

    void free(void* ptr) override {
        if(ptr < (void*)data && ptr >= (char*)data + BufferSize) {
            return;
        } else {
            size_t off = ((int_type*)ptr - data);
            data[off] = header.head;
            header.head = off;
        }
    }

};
