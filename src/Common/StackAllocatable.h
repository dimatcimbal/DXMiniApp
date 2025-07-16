//
// Created by dtcimbal on 15/07/2025.
#pragma once
#include <memory>

class StackAllocatable {
  protected:
    void* operator new(std::size_t) = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;
};
