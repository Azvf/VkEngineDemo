#pragma once

#include <optional>
#include <memory>
#include <vector>
#include <cassert>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "ShaderResource.h"

namespace Chandelier
{
    class VKContext;
}

