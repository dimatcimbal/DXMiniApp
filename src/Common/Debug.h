//
// Created by dtcimbal on 23/06/2025.
//
#pragma once

#include <windows.h> // Required for DEBUGPRINT
#include <cstdio>    // Required for swprintf_s

// Debug output macro
#define DEBUG_PRINT(msg, ...)                                                                       \
    do {                                                                                           \
        wchar_t buffer[256];                                                                       \
        swprintf_s(buffer, _countof(buffer), msg, ##__VA_ARGS__);                                  \
        OutputDebugString(buffer);                                                                 \
    } while (0)

// Helper macro for success messages
#define DEBUG_SUCCESS(msg, ...)                                                                    \
DEBUG_PRINT(L"[✅ DONE] " msg, ##__VA_ARGS__)

// Helper macro for error messages
#define DEBUG_ERROR(msg, ...)                                                                      \
DEBUG_PRINT(L"[❌ ERR] " msg, ##__VA_ARGS__)

// Helper macro for warning messages
#define DEBUG_WARN(msg, ...)                                                                       \
DEBUG_PRINT(L"[⚠️ WARN] " msg, ##__VA_ARGS__)

// Helper macro for info messages
#define DEBUG_INFO(msg, ...)                                                                       \
DEBUG_PRINT(L"[ℹ️ INFO] " msg, ##__VA_ARGS__)