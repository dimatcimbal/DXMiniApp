// src/Files/BaseFileProvider.h
// Created by dtcimbal on 2/06/2025.
#pragma once
#include <filesystem> // For std::filesystem::directory_iterator
#include <string>

// Represents a single file entry found during iteration.
struct FileEntry {
    std::wstring name;
};

// Custom iterator wrapper for std::filesystem::directory_iterator
class FileIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = FileEntry;
    using difference_type = std::ptrdiff_t;
    using pointer = const FileEntry*;
    using reference = const FileEntry&;

    FileIterator() = default;

    explicit FileIterator(std::filesystem::directory_iterator it) : m_it(std::move(it)) {
        if (m_it != std::filesystem::directory_iterator()) {
            m_currentEntry.name = m_it->path().filename().wstring();
        }
    }

    // Dereference operator
    FileEntry operator*() const {
        if (m_it == std::filesystem::directory_iterator()) {
            throw std::out_of_range("Dereferencing invalid FileIterator.");
        }
        return m_currentEntry;
    }

    // Pre-increment operator
    FileIterator& operator++() {
        if (m_it == std::filesystem::directory_iterator()) {
            throw std::out_of_range("Incrementing invalid FileIterator.");
        }
        ++m_it;
        if (m_it != std::filesystem::directory_iterator()) {
            m_currentEntry.name = m_it->path().filename().wstring();
        } else {
            // If we reached the end, clear the entry.
            m_currentEntry.name.clear();
        }
        return *this;
    }

    // Post-increment operator (optional, but good for completeness)
    FileIterator operator++(int) {
        FileIterator temp = *this;
        ++(*this);
        return temp;
    }

    // Equality comparison
    bool operator==(const FileIterator& other) const {
        return m_it == other.m_it;
    }

    // Inequality comparison
    bool operator!=(const FileIterator& other) const {
        return !(*this == other);
    }

  private:
    std::filesystem::directory_iterator m_it;
    FileEntry m_currentEntry; // Cache the current entry
};

class BaseFileProvider {
  public:
    explicit BaseFileProvider(std::filesystem::path directoryPath)
        : mDirectoryPath(std::move(directoryPath)) {
    }

    virtual ~BaseFileProvider() = default;

    // Returns an iterator to the beginning of the file collection.
    virtual FileIterator begin() = 0;

    // Returns an iterator to the end sentinel of the file collection.
    virtual FileIterator end() = 0;

    // Returns the display name of the directory being provided as a FileEntry.
    virtual FileEntry getCurrentDirectory() const = 0;

  protected:
    std::filesystem::path mDirectoryPath; // The directory path to be iterated
};