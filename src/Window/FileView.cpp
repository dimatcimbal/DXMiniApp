// src/Windows/FileView.cpp
#include "FileView.h"
#include <windows.h>  // For MultiByteToWideChar and MessageBoxW
#include <CommCtrl.h> // For TreeView and ListBox controls
#include <filesystem> // For file system operations
#include <string>     // For std::wstring
#include <vector>     // For std::vector

#include "Messages.h"

// Function to populate the tree view with the current folder and its files using
// std::filesystem
void Window::PopulateFileView(HWND hTreeView) {
    // Clear any existing items in the tree view before populating
    TreeView_DeleteAllItems(hTreeView);

    try {
        std::filesystem::path currentDir = std::filesystem::current_path();

        // Get the display name for the root node (current folder)
        // If the current directory is "C:\", filename() returns an empty string.
        // In that case, we'll use the root name (e.g., "C:\").
        std::wstring rootDisplayName =
            currentDir.filename().empty()
                ? currentDir.root_name().wstring() + currentDir.root_directory().wstring()
                : currentDir.filename().wstring();

        // Structure to insert the root item (current folder)
        TVITEMW tvItem{};
        tvItem.mask = TVIF_TEXT;
        tvItem.pszText = const_cast<LPWSTR>(rootDisplayName.c_str());

        TVINSERTSTRUCTW tvInsert{};
        tvInsert.hParent = TVI_ROOT;      // This item is a root item
        tvInsert.hInsertAfter = TVI_LAST; // Insert at the end
        tvInsert.item = tvItem;           // Assign the TVITEMW structure

        // Insert the root item and get its handle
        HTREEITEM hRoot = TreeView_InsertItem(hTreeView, &tvInsert);

        if (hRoot == NULL) {
            MessageBoxW(NULL, L"Failed to insert root item into TreeView.", L"Error",
                        MB_OK | MB_ICONERROR);
            return;
        }

        // Iterate through the directory to add files as children of the root node
        for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
            if (entry.is_regular_file()) { // Only add regular files
                // Get the filename as a wide string
                std::wstring fileName = entry.path().filename().wstring();

                // Structure to insert a child item (file)
                TVITEMW tvChildItem{}; // Explicitly use TVITEMW for wide characters
                tvChildItem.mask = TVIF_TEXT;
                tvChildItem.pszText = const_cast<LPWSTR>(fileName.c_str());

                TVINSERTSTRUCTW
                    tvChildInsert{};           // Explicitly use TVINSERTSTRUCTW for wide characters
                tvChildInsert.hParent = hRoot; // This item is a child of the root folder
                tvChildInsert.hInsertAfter = TVI_LAST; // Insert at the end
                tvChildInsert.item = tvChildItem;      // Assign the TVITEMW structure

                // Insert the file item
                TreeView_InsertItem(hTreeView, &tvChildInsert);
            }
        }

        // Explicitly expand the root node using SendMessage
        // This is often more reliable than just TreeView_Expand macro
        SendMessage(hTreeView, TVM_EXPAND, TVE_EXPAND, (LPARAM)hRoot);

        // Force a redraw of the TreeView to ensure expansion is visible immediately
        UpdateWindow(hTreeView);

    } catch (const std::filesystem::filesystem_error& e) {
        std::wstring errorMsg = L"Filesystem error: ";
        // Convert char* to wstring for MessageBoxW
        std::string narrowMsg = e.what();
        int bufferSize = MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, NULL, 0);
        if (bufferSize > 0) {
            std::vector<wchar_t> wBuf(bufferSize);
            MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, wBuf.data(), bufferSize);
            errorMsg += wBuf.data();
        }
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    } catch (const std::exception& e) {
        std::wstring errorMsg = L"General error: ";
        // Convert char* to wstring for MessageBoxW
        std::string narrowMsg = e.what();
        int bufferSize = MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, NULL, 0);
        if (bufferSize > 0) {
            std::vector<wchar_t> wBuf(bufferSize);
            MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, wBuf.data(), bufferSize);
            errorMsg += wBuf.data();
        }
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}

// New function to handle file selection logic from the TreeView
void Window::HandleFileSelection(HWND hTreeView,
                                 LPNMTREEVIEW lpnmtv,
                                 HWND hSceneView,
                                 HWND hSceneTree) {
    // Get the selected item's text
    TVITEMW tvItem;
    tvItem.mask = TVIF_TEXT;
    tvItem.hItem = lpnmtv->itemNew.hItem;

    // Allocate a buffer for the text
    wchar_t szText[MAX_PATH];
    tvItem.pszText = szText;
    tvItem.cchTextMax = MAX_PATH;

    if (TreeView_GetItem(hTreeView, &tvItem)) {
        std::wstring selectedItemText = szText;

        // Determine if the selected item is a file or a folder.
        // For simplicity, we'll assume only files are selectable as children
        // and the root is the current folder.
        // If you want to handle folder selection and change directory,
        // you'd add more logic here to differentiate.

        HTREEITEM hRoot = TreeView_GetRoot(hTreeView);
        // Only send file path if a child item is selected (not the root)
        if (lpnmtv->itemNew.hItem != hRoot) {
            std::filesystem::path currentPath = std::filesystem::current_path();
            std::filesystem::path fullPath = currentPath / selectedItemText;
            std::wstring fullPathStr = fullPath.native();

            // Send message with full path to the scene views
            SendMessageW(hSceneView, WM_APP_FILE_SELECTED, 0, (LPARAM)fullPathStr.c_str());
            SendMessageW(hSceneTree, WM_APP_FILE_SELECTED, 0, (LPARAM)fullPathStr.c_str());

            OutputDebugStringW((L"Selected File: " + fullPathStr + L"\n").c_str());
        } else {
            OutputDebugStringW((L"Selected Folder (Root): " + selectedItemText + L"\n").c_str());
        }
    }
}
