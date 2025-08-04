cmake_minimum_required(VERSION 3.15)

# Fail early if not on Windows
if(NOT WIN32)
    message(FATAL_ERROR "This project is designed for Windows only")
endif()

project(DXMiniApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
elseif(MSVC)
    add_compile_options(/W4)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INC_DIR}/*.h" "${INC_DIR}/*.hpp")

# VCPKG dependencies
find_package(directx-headers CONFIG REQUIRED)
find_path(D3DX12_INCLUDE_DIR "d3dx12.h")

# Create a Windows application (not console)
add_executable(DXMiniApp WIN32 ${SOURCES} ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/app.rc)

# Define UNICODE and _UNICODE preprocessor macros for the project.
# This makes TCHAR and related WinAPI functions resolve to their wide-character (W) versions.
target_compile_definitions(DXMiniApp PRIVATE UNICODE _UNICODE)

# Include directories
target_include_directories(DXMiniApp PRIVATE
    ${INC_DIR}
    ${SRC_DIR}
    ${D3DX12_INCLUDE_DIR}
)

# Link required Windows libraries
target_link_libraries(DXMiniApp
    PRIVATE
        user32      # User interface functions
        gdi32       # Graphics device interface
        kernel32    # Core Windows functions
        gdiplus     # GDI+ for anti-aliasing fonts
        comctl32    # Common Controls (TreeView, ListBox, InitCommonControlsEx)
        shlwapi     # Shell Light-weight Utility library (PathCombine, PathFindFileName)
        d3d12       # Direct3D 12 API
        dxgi        # DirectX Graphics Infrastructure
        dxguid      # DirectX GUID definitions
        Microsoft::DirectX-Headers # VCPKG's include directories for directx-headers
)

# Set subsystem to Windows (removes console window)
set_target_properties(DXMiniApp PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
)

# MSVC-specific subsystem setting
if(MSVC)
    set_target_properties(DXMiniApp PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# Optional: Set working directory for debugging in Visual Studio
set_target_properties(DXMiniApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

install(TARGETS DXMiniApp
    RUNTIME DESTINATION bin
)