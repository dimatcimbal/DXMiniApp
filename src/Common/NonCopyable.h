//
// Created by dtcimbal on 15/07/2025.
#pragma once
class NonCopyable {
  protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
