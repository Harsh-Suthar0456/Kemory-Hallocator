## Directory Format

1. ./include
    - public includes

2. ./src
    - files for each of the classes etc(buddy general segregated)
3. ./tests
    - Tests for each of the classes

## Classes
- Top level buddy allocator to assign space to each of the lower level allocators
- Embedded linked list
- General allocator 
- Segregated list for custom size objects(object size provided on initialization)
- User side common interface

## Class schema 
Each class is supposed to inherit a base class to start with, named Allocator 


### Class Allocator Schema
```c++
class Allocator{
    void* get(size_t size);
    void free(void* ptr);
};
```


