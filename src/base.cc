#include "base.h"
#include "segregated.h"

void *AllocatorBlock::get(size_t size) {
  switch (type) {
  case AllocatorType::SegregatedList1:
    return getAs<SegregatedListBlock<1>>(size);
  case AllocatorType::SegregatedList2:
    return getAs<SegregatedListBlock<2>>(size);
  case AllocatorType::SegregatedList4:
    return getAs<SegregatedListBlock<4>>(size);
  case AllocatorType::SegregatedList8:
    return getAs<SegregatedListBlock<8>>(size);
  case AllocatorType::SegregatedList16:
    return getAs<SegregatedListBlock<16>>(size);
  default:
    return nullptr;
  }
}

void AllocatorBlock::free(void *ptr) {
  switch (type) {
  case AllocatorType::SegregatedList1:
    freeAs<SegregatedListBlock<1>>(ptr);
  case AllocatorType::SegregatedList2:
    freeAs<SegregatedListBlock<2>>(ptr);
  case AllocatorType::SegregatedList4:
    freeAs<SegregatedListBlock<4>>(ptr);
  case AllocatorType::SegregatedList8:
    freeAs<SegregatedListBlock<8>>(ptr);
  case AllocatorType::SegregatedList16:
    freeAs<SegregatedListBlock<16>>(ptr);
  default:
  }
}