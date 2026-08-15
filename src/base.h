#pragma once 

#include <cstddef>

class Allocator { 
  public: 
    virtual void* get(size_t size) = 0;
    virtual void free(void* ptr) = 0;  
};