#pragma once
#include <windows.h>
#include <string> // For std::wstring

namespace Window {
// Function to populate a ListBox with files from the current directory
void PopulateFileList(HWND hListBox);

} // namespace Window