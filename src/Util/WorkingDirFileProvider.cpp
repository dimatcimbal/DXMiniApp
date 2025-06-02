#include "WorkingDirFileProvider.h"
#include <windows.h>
#include <filesystem>

namespace Util {

// --- WorkingDirFileProvider Implementation ---

FileIterator WorkingDirFileProvider::begin() {
    try {
        // Construct a FileIterator from a std::filesystem::directory_iterator
        return FileIterator(std::filesystem::directory_iterator(m_directoryPath));
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle cases where directory cannot be opened (e.g., permissions)
        OutputDebugStringW((L"Filesystem error in WorkingDirFileProvider::begin: " +
                            std::filesystem::path(e.what()).wstring() + L"\n")
                               .c_str());
        // Return an end sentinel
        return FileIterator();
    }
}

FileIterator WorkingDirFileProvider::end() {
    return FileIterator(std::filesystem::directory_iterator());
}

} // namespace Util