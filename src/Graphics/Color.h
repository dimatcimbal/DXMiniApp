//
// Created by dtcimbal on 18/07/2025.
#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Color {
  public:
    Color() : mValue(g_XMOne) {
    }
    Color(const XMVECTORF32& vec);
    float R() const {
        return XMVectorGetX(mValue);
    }
    float G() const {
        return XMVectorGetY(mValue);
    }
    float B() const {
        return XMVectorGetZ(mValue);
    }
    float A() const {
        return XMVectorGetW(mValue);
    }

  private:
    XMVECTORF32 mValue;
};
