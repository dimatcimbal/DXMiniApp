#include "NodeView.h"
#include <CommCtrl.h> // For TVINSERTSTRUCT (if not already included by Windows.h)

void LoadSceneIntoTree(HWND hTreeView, const std::wstring& filePath) {
    // Clear existing items in the tree view
    TreeView_DeleteAllItems(hTreeView);

    // Add a root item (e.g., the selected file name)
    TVITEMW tvItem{};
    tvItem.mask = TVIF_TEXT;
    tvItem.pszText = (LPWSTR)filePath.c_str(); // Cast const to non-const for TVITEMW

    TVINSERTSTRUCTW tvInsert{};
    tvInsert.hParent = TVI_ROOT;
    tvInsert.hInsertAfter = TVI_LAST;
    tvInsert.item = tvItem;

    HTREEITEM hRoot = TreeView_InsertItem(hTreeView, &tvInsert);

    // Add some dummy child items for demonstration
    if (hRoot) {
        tvItem.pszText = (LPWSTR)L"Model 1";
        tvInsert.hParent = hRoot;
        TreeView_InsertItem(hTreeView, &tvInsert);

        tvItem.pszText = (LPWSTR)L"Camera";
        TreeView_InsertItem(hTreeView, &tvInsert);

        tvItem.pszText = (LPWSTR)L"Light Source";
        TreeView_InsertItem(hTreeView, &tvInsert);
    }
    // Expand the root item
    TreeView_Expand(hTreeView, hRoot, TVE_EXPAND);
}