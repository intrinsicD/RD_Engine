//vulkan/VulkanHelper.h
#pragma once

#include "ral/Device.h"
#include "VulkanDevice.h"

namespace RDE {
    struct VulkanHelper {
        explicit VulkanHelper(RAL::Device *device) : m_Device(dynamic_cast<VulkanDevice *>(device)) {}

        VulkanDevice *m_Device = nullptr;
    };
}