// src/Util/WorkingDirFileProvider.cpp
// Created by dtcimbal on 2/06/2025.
#include "WorkingDirFileProvider.h"
#include <windows.h>
#include <filesystem>

#include "../Util/Debug.h"

// --- WorkingDirFileProvider Implementation ---

FileIterator WorkingDirFileProvider::begin() {
    try {
        // Construct a FileIterator from a std::filesystem::directory_iterator
        return FileIterator(std::filesystem::directory_iterator(mDirectoryPath));
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle cases where directory cannot be opened (e.g., permissions)
        DEBUGPRINT(L"Filesystem error in WorkingDirFileProvider::begin: %s.\n",
                   std::filesystem::path(e.what()).wstring().c_str());
        // Return an end sentinel
        return FileIterator();
    }
}

FileIterator WorkingDirFileProvider::end() {
    return FileIterator(std::filesystem::directory_iterator());
}

FileEntry WorkingDirFileProvider::getCurrentDirectory() const {
    // Construct a FileEntry from the directory path's display name
    FileEntry entry;
    entry.name = mDirectoryPath.filename().empty() ? mDirectoryPath.root_path().wstring() +
                                                         mDirectoryPath.root_directory().wstring()
                                                   : mDirectoryPath.filename().wstring();
    return entry;
}
