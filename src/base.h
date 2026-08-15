#pragma once 

#include <cstddef>

class Allocator { 
  public: 
    void* get(size_t size);
    void free(void* ptr);  
};