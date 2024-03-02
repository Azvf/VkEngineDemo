#pragma once

#include "runtime/core/math/vector3.h"

namespace Chandelier
{
    inline constexpr int32_t kMaxPointLightCount = 8;

    struct PointLight
    {
        Vector3 position;
        float   radius;
        Vector3 color;
        float   intensity;
    };

    struct Lights
    {
        PointLight point_lights[kMaxPointLightCount];
        int32_t    point_light_num;

        glm::vec3 padding_0;
        glm::vec4 padding_1;
        glm::vec4 padding_2;
        glm::vec4 padding_3;
    };

} // namespace Chandelier