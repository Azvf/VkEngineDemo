#pragma once

#include "VkCommon.h"

namespace Chandelier
{
    enum GPUSamplerExtendMode : uint8_t
    {
        /**
         * Extrapolate by extending the edge pixels of the texture, in other words, the texture
         * coordinates are clamped.
         */
        GPU_SAMPLER_EXTEND_MODE_EXTEND = 0,
        /** Extrapolate by repeating the texture. */
        GPU_SAMPLER_EXTEND_MODE_REPEAT,
        /** Extrapolate by repeating the texture with mirroring in a ping-pong fashion. */
        GPU_SAMPLER_EXTEND_MODE_MIRRORED_REPEAT,
        /**
         * Extrapolate using the value of TEXTURE_BORDER_COLOR, which is always set to a transparent
         * black color (0, 0, 0, 0) and can't be changed.
         */
        GPU_SAMPLER_EXTEND_MODE_CLAMP_TO_BORDER,
    };

    enum GPUSamplerFiltering : uint8_t
    {
        /**
         * Default sampler filtering with all options off.
         * It means no linear filtering, no mipmapping, and no anisotropic filtering.
         */
        GPU_SAMPLER_FILTERING_DEFAULT = 0,
        /**
         * Enables hardware linear filtering.
         * Also enables linear interpolation between MIPS if GPU_SAMPLER_FILTERING_MIPMAP is set.
         */
        GPU_SAMPLER_FILTERING_LINEAR = (1 << 0),
        /**
         * Enables mipmap access through shader samplers.
         * Also enables linear interpolation between mips if GPU_SAMPLER_FILTER is set, otherwise the mip
         * interpolation will be set to nearest.
         *
         * The following parameters are always left to their default values and can't be changed:
         * - TEXTURE_MIN_LOD is -1000.
         * - TEXTURE_MAX_LOD is 1000.
         * - TEXTURE_LOD_BIAS is 0.0f.
         */
        GPU_SAMPLER_FILTERING_MIPMAP = (1 << 1),
        /**
         * Enable Anisotropic filtering. This only has effect if `GPU_SAMPLER_FILTERING_MIPMAP` is set.
         * The filtered result is implementation dependent.
         *
         * The maximum amount of samples is always set to its maximum possible value and can't be
         * changed, except by the user through the user preferences, see the use of U.anisotropic_filter.
         */
        GPU_SAMPLER_FILTERING_ANISOTROPIC = (1 << 2),
    };

    enum GPUSamplerStateType : uint8_t
    {
        /**
         * The filtering, extend_x, and extend_yz members of the GPUSamplerState structure will be used
         * in setting up the sampler state for the texture. The custom_type member will be ignored in
         * that case.
         */
        GPU_SAMPLER_STATE_TYPE_PARAMETERS = 0,
        /**
         * The filtering, extend_x, and extend_yz members of the GPUSamplerState structure will be
         * ignored, and the predefined custom parameters outlined in the documentation of
         * GPUSamplerCustomType will be used in setting up the sampler state for the texture.
         */
        GPU_SAMPLER_STATE_TYPE_CUSTOM,
        /**
         * The members of the GPUSamplerState structure will be ignored and the internal sampler state of
         * the texture will be used. In other words, this is a signal value and stores no useful or
         * actual data.
         */
        GPU_SAMPLER_STATE_TYPE_INTERNAL,
    };

    enum GPUSamplerCustomType : uint8_t
    {
        /**
         * Enable compare mode for depth texture. The depth texture must then be bound to a shadow
         * sampler. This is equivalent to:
         *
         * - GPU_SAMPLER_FILTERING_LINEAR.
         * - GPU_SAMPLER_EXTEND_MODE_EXTEND.
         *
         * And sets:
         *
         * - TEXTURE_COMPARE_MODE -> COMPARE_REF_TO_TEXTURE.
         * - TEXTURE_COMPARE_FUNC -> LEQUAL.
         */
        GPU_SAMPLER_CUSTOM_COMPARE = 0,
        /**
         * Special icon sampler with custom LOD bias and interpolation mode. This sets:
         *
         * - TEXTURE_MAG_FILTER -> LINEAR.
         * - TEXTURE_MIN_FILTER -> LINEAR_MIPMAP_NEAREST.
         * - TEXTURE_LOD_BIAS   -> -0.5.
         */
        GPU_SAMPLER_CUSTOM_ICON,
    };

    static const int GPU_SAMPLER_EXTEND_MODES_COUNT = GPU_SAMPLER_EXTEND_MODE_CLAMP_TO_BORDER + 1;

    static const int GPU_SAMPLER_FILTERING_TYPES_COUNT =
        (GPU_SAMPLER_FILTERING_LINEAR | GPU_SAMPLER_FILTERING_MIPMAP | GPU_SAMPLER_FILTERING_ANISOTROPIC) + 1;

    struct GPUSamplerState
    {
        /** Specifies the enabled filtering options for the sampler. */
        GPUSamplerFiltering filtering : 8;
        /**
         * Specifies how the texture will be extrapolated for out-of-bound texture sampling along the x
         * axis.
         */
        GPUSamplerExtendMode extend_x : 4;
        /**
         * Specifies how the texture will be extrapolated for out-of-bound texture sampling along both
         * the y and z axis. There is no individual control for the z axis because 3D textures have
         * limited use, and when used, their extend mode is typically the same for all axis.
         */
        GPUSamplerExtendMode extend_yz : 4;
        /** Specifies the type of sampler if the state type is GPU_SAMPLER_STATE_TYPE_CUSTOM. */
        GPUSamplerCustomType custom_type : 8;
        /** Specifies how the GPUSamplerState structure should be interpreted when passed around. */
        GPUSamplerStateType type : 8;

        /**
         * Constructs a sampler state with default filtering and extended extend in both x and y axis.
         * See the documentation on GPU_SAMPLER_FILTERING_DEFAULT and GPU_SAMPLER_EXTEND_MODE_EXTEND for
         * more information.
         *
         * GPU_SAMPLER_STATE_TYPE_PARAMETERS is set in order to utilize the aforementioned parameters, so
         * GPU_SAMPLER_CUSTOM_COMPARE is arbitrary, ignored, and irrelevant.
         */
        static constexpr GPUSamplerState default_sampler()
        {
            return {GPU_SAMPLER_FILTERING_DEFAULT,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_CUSTOM_COMPARE,
                    GPU_SAMPLER_STATE_TYPE_PARAMETERS};
        }

        /**
         * Constructs a sampler state that can be used to signal that the internal sampler of the texture
         * should be used instead. See the documentation on GPU_SAMPLER_STATE_TYPE_INTERNAL for more
         * information.
         *
         * GPU_SAMPLER_STATE_TYPE_INTERNAL is set in order to signal the use of the internal sampler of
         * the texture, so the rest of the options before it are arbitrary, ignored, and irrelevant.
         */
        static constexpr GPUSamplerState internal_sampler()
        {
            return {GPU_SAMPLER_FILTERING_DEFAULT,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_CUSTOM_COMPARE,
                    GPU_SAMPLER_STATE_TYPE_INTERNAL};
        }

        /**
         * Constructs a special sampler state that can be used sampler icons. See the documentation on
         * GPU_SAMPLER_CUSTOM_ICON for more information.
         *
         * GPU_SAMPLER_STATE_TYPE_CUSTOM is set in order to specify a custom sampler type, so the rest of
         * the options before it are arbitrary, ignored, and irrelevant.
         */
        static constexpr GPUSamplerState icon_sampler()
        {
            return {GPU_SAMPLER_FILTERING_DEFAULT,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_CUSTOM_ICON,
                    GPU_SAMPLER_STATE_TYPE_CUSTOM};
        }

        /**
         * Constructs a special sampler state for depth comparison. See the documentation on
         * GPU_SAMPLER_CUSTOM_COMPARE for more information.
         *
         * GPU_SAMPLER_STATE_TYPE_CUSTOM is set in order to specify a custom sampler type, so the rest of
         * the options before it are ignored and irrelevant, but they are set to sensible defaults in
         * case comparison mode is turned off, in which case, the sampler state will become equivalent to
         * GPUSamplerState::default_sampler().
         */
        static constexpr GPUSamplerState compare_sampler()
        {
            return {GPU_SAMPLER_FILTERING_DEFAULT,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_EXTEND_MODE_EXTEND,
                    GPU_SAMPLER_CUSTOM_COMPARE,
                    GPU_SAMPLER_STATE_TYPE_CUSTOM};
        }
    };

    static VkSamplerAddressMode CvtToVkSamplerAddrMode(const GPUSamplerExtendMode extend_mode)
    {
        switch (extend_mode)
        {
            case GPU_SAMPLER_EXTEND_MODE_EXTEND:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case GPU_SAMPLER_EXTEND_MODE_REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case GPU_SAMPLER_EXTEND_MODE_MIRRORED_REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case GPU_SAMPLER_EXTEND_MODE_CLAMP_TO_BORDER:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }

        assert(0);
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

} // namespace Chandelier