#pragma once

#include "runtime/core/math/vector2.h"

namespace Chandelier
{
    struct ScreenMesh
    {
        Vector2 position;
        Vector2 uv;
    };

    // @todo: overhaul temp code
    // vertex attributes for a quad that fills the entire screen in normalized Device Coordinates.
    const std::vector<float> ScreenVertices = { 
        // positions        // texCoords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

} // namespace Chandelier
