#pragma once 
#include <cstddef>

[[nodiscard]] void* khalloc(size_t sz);
void free(void* ptr);
