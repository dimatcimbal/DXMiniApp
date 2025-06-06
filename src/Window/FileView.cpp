// src/Window/FileView.cpp
// Created by dtcimbal on 26/05/2025.
#include "FileView.h"
#include <commctrl.h> // Required for ListView functions (e.g., ListView_InsertItem)
#include <filesystem>
#include "Util/WorkingDirFileProvider.h"

// Link with Comctl32.lib for common controls
#pragma comment(lib, "Comctl32.lib")

FileView::FileView(Util::BaseFileProvider& fileProvider) : mFileProvider(fileProvider) {
}

FileView::~FileView() = default;

// Creates the ListView control for the file view.
bool FileView::OnCreate(HWND hParent, UINT id) {
    // Initialize common controls once.
    // This is often done globally in WinMain, but placing it here ensures it's done before
    // creating a ListView if this component is instantiated independently.
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES; // Corrected: Removed the negation
    InitCommonControlsEx(&icex);

    mHWnd = CreateWindowEx(WS_EX_CLIENTEDGE, // Extended style for a sunken border
                           WC_TREEVIEW,      // Tree View control class name (from commctrl.h) -
                                        // Corrected comment: It's a TreeView, not ListView here.
                           nullptr, // Window text (none for a list view)
                           WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |
                               TVS_SHOWSELALWAYS, // TreeView styles for file view
                           0, 0, 0, 0,            // Initial position and size will be set by parent
                           hParent,               // Parent window
                           (HMENU)(INT_PTR)id,    // Child window ID
                           GetModuleHandle(nullptr), nullptr);

    if (mHWnd == nullptr) {
        OutputDebugString(L"Failed to create FileView TreeView.\n"); // Corrected message
        return false;
    }

    PopulateFileView(); // Populate it immediately after creation

    return true;
}

// Populates the file view control with the files within the same dir.
void FileView::PopulateFileView() {
    // Clear any existing items in the tree view before populating
    TreeView_DeleteAllItems(mHWnd);

    try {
        // Get the current directory as a FileEntry directly from the file provider
        Util::FileEntry rootEntry = mFileProvider.getCurrentDirectory();
        std::wstring rootDisplayName = rootEntry.name;

        // Structure to insert the root item (current folder)
        TVITEMW tvItem{};
        tvItem.mask = TVIF_TEXT;
        tvItem.pszText = const_cast<LPWSTR>(rootDisplayName.c_str());

        TVINSERTSTRUCTW tvInsert{};
        tvInsert.hParent = TVI_ROOT;      // This item is a root item
        tvInsert.hInsertAfter = TVI_LAST; // Insert at the end
        tvInsert.item = tvItem;           // Assign the TVITEMW structure

        // Insert the root item and get its handle
        HTREEITEM hRoot = TreeView_InsertItem(mHWnd, &tvInsert);

        if (hRoot == nullptr) {
            MessageBoxW(nullptr, L"Failed to insert root item into TreeView.", L"Error",
                        MB_OK | MB_ICONERROR);
            return;
        }

        // Iterate through the directory to add files as children of the root node
        for (const auto& entry : mFileProvider) {
            // Structure to insert a child item (file)
            TVITEMW tvChildItem{}; // Explicitly use TVITEMW for wide characters
            tvChildItem.mask = TVIF_TEXT;
            tvChildItem.pszText = const_cast<LPWSTR>(entry.name.c_str());

            TVINSERTSTRUCTW
            tvChildInsert{};               // Explicitly use TVINSERTSTRUCTW for wide characters
            tvChildInsert.hParent = hRoot; // This item is a child of the root folder
            tvChildInsert.hInsertAfter = TVI_LAST; // Insert at the end
            tvChildInsert.item = tvChildItem;      // Assign the TVITEMW structure

            // Insert the file item
            TreeView_InsertItem(mHWnd, &tvChildInsert);
        }

        // Explicitly expand the root node using SendMessage
        // This is often more reliable than just TreeView_Expand macro
        SendMessage(mHWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)hRoot);

        // Force a redraw of the TreeView to ensure expansion is visible immediately
        UpdateWindow(mHWnd);

    } catch (const std::filesystem::filesystem_error& e) {
        std::wstring errorMsg = L"Filesystem error: ";
        // Convert char* to wstring for MessageBoxW
        std::string narrowMsg = e.what();
        int bufferSize = MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, nullptr, 0);
        if (bufferSize > 0) {
            std::vector<wchar_t> wBuf(bufferSize);
            MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, wBuf.data(), bufferSize);
            errorMsg += wBuf.data();
        }
        MessageBoxW(nullptr, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    } catch (const std::exception& e) {
        std::wstring errorMsg = L"General error: ";
        // Convert char* to wstring for MessageBoxW
        std::string narrowMsg = e.what();
        int bufferSize = MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, nullptr, 0);
        if (bufferSize > 0) {
            std::vector<wchar_t> wBuf(bufferSize);
            MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, wBuf.data(), bufferSize);
            errorMsg += wBuf.data();
        }
        MessageBoxW(nullptr, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}