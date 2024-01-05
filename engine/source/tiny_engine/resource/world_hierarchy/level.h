#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/core/math/vector3.h"

#include "resource/world_hierarchy/object.h"

namespace Chandelier
{
    REFLECTION_TYPE(LevelRes)
    CLASS(LevelRes, Fields)
    {
        REFLECTION_BODY(LevelRes);

    public:
        Vector3     m_gravity {0.f, 0.f, -9.8f};
        std::string m_character_name;

        std::vector<ObjectInstanceRes> m_objects;
    };
} // namespace Chandelier