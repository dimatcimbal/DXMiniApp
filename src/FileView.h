#pragma once
#include <windows.h>
#include <string> // For std::wstring

// Function to populate a ListBox with files from the current directory
void PopulateFileList(HWND hListBox);
