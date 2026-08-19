#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

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

#ifdef __SIZEOF_INT128__
using uint128_t = __uint128_t;
#endif

template <> struct unsigned_integral<16> {
  using type = uint128_t;
};

template <size_t Bytes>
using unsigned_integral_t = unsigned_integral<Bytes>::type;
} // namespace

// Placeholder globally accessible buddy allocator get/free functions
void *buddy_get(size_t size);
void buddy_free(void *ptr);

#pragma pack(push, 1)

template <size_t Bytes> class SegregatedListBlock : AllocatorBlock {
public:
  using int_type = unsigned_integral_t<Bytes>;

  struct Header {
    int_type head;
    int_type used;
    SegregatedListBlock *next;
  };

  constexpr static size_t MaxCount = std::max(1uz << Bytes, PAGE_SIZE / Bytes);
  constexpr static size_t BlockSize = MaxCount * Bytes;
  constexpr static size_t MetadataSize = sizeof(Header) + sizeof(AllocatorBlock);
  constexpr static size_t BufferSize =
      BlockSize - MetadataSize;
  constexpr static size_t BufferCount = BufferSize / Bytes;
  constexpr static size_t Padding = BufferSize - BufferCount * Bytes; 

private:
  Header header;
  char padding[Padding];
  int_type data[BufferCount];

public:
  SegregatedListBlock()
      : header({std::numeric_limits<int_type>::max(), 0, nullptr}) {}

  static SegregatedListBlock *newAllocator() {
    void* newBlockBuffer = buddy_get(BlockSize);
    SegregatedListBlock* newBlock = new (newBlockBuffer) SegregatedListBlock;
    return newBlock;
  }

  void *get(size_t size) {
    if (size != Bytes) [[unlikely]] {
      return nullptr;
    }
    if (header.head >= BufferCount) {
      if (header.used < BufferCount) [[likely]] {
        int_type offset = header.used++;
        return &data[offset];
      } else {
        if (!header.next) {
          header.next = newAllocator();
        }
        return header.next->get(size);
        return nullptr;
      }
    } else {
      int_type temp = header.head;
      header.head = data[header.head];
      return &data[temp];
    }
  }

  void free(void *ptr) {
    if (ptr < (void *)data || ptr >= (char *)data + BufferSize) {
      return;
    }
    size_t off = ((int_type *)ptr - data);
    data[off] = header.head;
    header.head = off;
  }
};

#pragma pack(pop)
