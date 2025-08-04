//
// Created by dtcimbal on 27/07/2025.
#pragma once
#include <memory>
#include "Scene/Camera.h"

class Renderer {
  public:
    bool OnResize(uint32_t NewWidth, uint32_t NewHeight);
    bool Draw(Camera& Camera);
};
