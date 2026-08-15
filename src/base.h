#pragma once 

#include <cstddef>

constexpr size_t PAGE_SIZE = 4096;

class Allocator { 
  public: 
    virtual void* get(size_t size) = 0;
    virtual void free(void* ptr) = 0;  
};