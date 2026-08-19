#include "base.h"
#include "segregated.h"

void* AllocatorBlock::get(size_t size) {
  switch (type) {
  case AllocatorType::SegregatedList1:
    return reinterpret_cast<SegregatedListBlock<1> *>(this)->get(size);
  case AllocatorType::SegregatedList2:
    return reinterpret_cast<SegregatedListBlock<2> *>(this)->get(size);
  case AllocatorType::SegregatedList4:
    return reinterpret_cast<SegregatedListBlock<4> *>(this)->get(size);
  case AllocatorType::SegregatedList8:
    return reinterpret_cast<SegregatedListBlock<8> *>(this)->get(size);
  case AllocatorType::SegregatedList16:
    return reinterpret_cast<SegregatedListBlock<16> *>(this)->get(size);
  default:
    return nullptr;
  }
}

void AllocatorBlock::free(void *ptr) {
  switch (type) {
  case AllocatorType::SegregatedList1:
    reinterpret_cast<SegregatedListBlock<1> *>(this)->free(ptr);
  case AllocatorType::SegregatedList2:
    reinterpret_cast<SegregatedListBlock<2> *>(this)->free(ptr);
  case AllocatorType::SegregatedList4:
    reinterpret_cast<SegregatedListBlock<4> *>(this)->free(ptr);
  case AllocatorType::SegregatedList8:
    reinterpret_cast<SegregatedListBlock<8> *>(this)->free(ptr);
  case AllocatorType::SegregatedList16:
    reinterpret_cast<SegregatedListBlock<16> *>(this)->free(ptr);
  default:
  }
}