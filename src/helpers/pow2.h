//
// Created by Peter Eichinger on 04.10.18.
//

#pragma once
#include <cmath>
#include <vulkan/vulkan.hpp>
vk::DeviceSize nextPowerOfTwo(vk::DeviceSize value) {
    return static_cast<vk::DeviceSize>(std::pow(2, std::ceil(std::log2(value))));
}