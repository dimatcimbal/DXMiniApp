// Helper header to configure and include necessary DirectX headers.
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h> // get the latest

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif
