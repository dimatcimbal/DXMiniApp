﻿// src/Window/SceneTree.cpp
#include "SceneTree.h"
#include <commctrl.h> // Required for TreeView functions (e.g., TreeView_InsertItem)

SceneTree::SceneTree() = default;

SceneTree::~SceneTree() = default;

// Creates the TreeView control for the scene tree.
bool SceneTree::OnCreate(HWND hParent, UINT id) {
    // Initialize common controls (TreeView specific)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES; // Initialize TreeView control class
    InitCommonControlsEx(&icex);

    m_hWnd =
        CreateWindowEx(WS_EX_CLIENTEDGE,
                       WC_TREEVIEW, // Tree View control class name
                       L"", WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT,
                       0, 0, 0, 0, // Position and size will be set by parent
                       hParent, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);

    if (m_hWnd == nullptr) {
        OutputDebugString(L"Failed to create SceneTree TreeView.\n");
        return false;
    }

    AddDummyNodes(); // Populate with some dummy data

    return true;
}

void SceneTree::AddDummyNodes() {
    if (m_hWnd == nullptr)
        return;

    TVINSERTSTRUCT tvInsert;
    tvInsert.hParent = TVI_ROOT;
    tvInsert.hInsertAfter = TVI_LAST;
    tvInsert.item.mask = TVIF_TEXT;

    tvInsert.item.pszText = (LPWSTR)L"Root Node";
    HTREEITEM hRoot = TreeView_InsertItem(m_hWnd, &tvInsert);

    if (hRoot) {
        tvInsert.hParent = hRoot;
        tvInsert.item.pszText = (LPWSTR)L"Child Node 1";
        HTREEITEM hChild1 = TreeView_InsertItem(m_hWnd, &tvInsert);

        if (hChild1) {
            tvInsert.hParent = hChild1;
            tvInsert.item.pszText = (LPWSTR)L"Grandchild Node A";
            TreeView_InsertItem(m_hWnd, &tvInsert);
        }

        tvInsert.hParent = hRoot;
        tvInsert.item.pszText = (LPWSTR)L"Child Node 2";
        TreeView_InsertItem(m_hWnd, &tvInsert);
    }
}