#pragma once

#include <filesystem>
#include "BaseFileProvider.h"

namespace Util {

// Provides access to files within the current working directory, supporting range-based for loops.
class WorkingDirFileProvider : public BaseFileProvider {
  public:
    // Constructor now calls the base class constructor with std::filesystem::current_path()
    WorkingDirFileProvider() : BaseFileProvider(std::filesystem::current_path()) {
    }
    ~WorkingDirFileProvider() = default;

    FileIterator begin() override;
    FileIterator end() override;
    FileEntry getCurrentDirectory() const override;
};

} // namespace Util