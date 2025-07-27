//
// Created by dtcimbal on 27/07/2025.
#include "Device.h"

bool Device::CreateRenderer(std::unique_ptr<Renderer>& OutRenderer) {
    // TODO handle device creation logic here
    OutRenderer = std::make_unique<Renderer>();
    return true;
}
