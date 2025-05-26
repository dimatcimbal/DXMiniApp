#include "FileView.h"
#include <CommCtrl.h> // For TreeView and ListBox controls
#include <filesystem> // For file system operations

// Function to populate the file list box with files from the current directory using
// std::filesystem
void Window::PopulateFileList(HWND hListBox) {
    try {
        std::filesystem::path currentDir = std::filesystem::current_path();
        for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
            if (entry.is_regular_file()) { // Only add regular files
                // SendMessage expects LPCTSTR, which resolves to const char* or const wchar_t*
                // Since _T is used, it will be wchar_t* in Unicode builds, matching
                // std::filesystem::path::c_str() native()
                SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)entry.path().filename().c_str());
            }
        }
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
