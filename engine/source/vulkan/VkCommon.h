#pragma once

#include <optional>
#include <memory>
#include <vector>
#include <cassert>
#include <array>
#include <string>
#include <atomic>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "ShaderResource.h"

namespace Chandelier
{
    using Vector2i = glm::ivec2;

    class VKContext;
    class Texture;
    class Buffer;
    class Mesh;
}

