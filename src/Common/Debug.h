//
// Created by dtcimbal on 23/06/2025.
//
#pragma once

#include <windows.h> // Required for DEBUGPRINT
#include <cstdio>    // Required for swprintf_s

#define DEBUGPRINT(msg, ...)                                                                       \
    do {                                                                                           \
        wchar_t buffer[256];                                                                       \
        swprintf_s(buffer, _countof(buffer), msg, __VA_ARGS__);                                    \
        OutputDebugString(buffer);                                                                 \
    } while (0)