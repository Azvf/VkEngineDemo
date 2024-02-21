//#pragma once
//
//#include "VkCommon.h"
//#include "Texture.h"
//#include "Descriptor.h"
//
//namespace Chandelier
//{
//    enum StorageType : uint8_t
//    {
//        None_Type = 0,
//        Push_Constants,
//        Uniform_Buffer,
//    };
//
//    struct Layout
//    {
//        static constexpr StorageType STORAGE_TYPE_DEFAULT  = StorageType::PUSH_CONSTANTS;
//        static constexpr StorageType STORAGE_TYPE_FALLBACK = StorageType::UNIFORM_BUFFER;
//
//        struct PushConstant
//        {
//            /* Used as lookup based on ShaderInput. */
//            int32_t location;
//
//            /** Offset in the push constant data (in bytes). */
//            uint32_t     offset;
//            shader::Type type;
//            int          array_size;
//        };
//
//    private:
//        std::vector<PushConstant> push_constants;
//        uint32_t             size_in_bytes_ = 0;
//        StorageType          storage_type_  = StorageType::None_Type;
//        /**
//         * Binding index in the descriptor set when the push constants use an uniform buffer.
//         */
//        Location descriptor_set_location_;
//
//    public:
//        /**
//         * Return the desired storage type that can fit the push constants of the given shader
//         * create info, matching the limits of the given device.
//         *
//         * Returns:
//         * - StorageType::NONE: No push constants are needed.
//         * - StorageType::PUSH_CONSTANTS: Regular vulkan push constants can be used.
//         * - StorageType::UNIFORM_BUFFER: The push constants don't fit in the limits of the
//         * given device. A uniform buffer should be used as a fallback method.
//         */
//        static StorageType determine_storage_type(const shader::ShaderCreateInfo& info,
//                                                  const VKDevice&                 device);
//
//        /**
//         * Initialize the push constants of the given shader create info with the
//         * binding location.
//         *
//         * interface: Uniform locations of the interface are used as lookup key.
//         * storage_type: The type of storage for push constants to use.
//         * location: When storage_type=StorageType::UNIFORM_BUFFER this contains
//         *    the location in the descriptor set where the uniform buffer can be
//         *    bound.
//         */
//        void init(const shader::ShaderCreateInfo& info,
//                  const VKShaderInterface&        interface,
//                  StorageType                     storage_type,
//                  Location       location);
//
//        /**
//         * Return the storage type that is used.
//         */
//        StorageType storage_type_get() const { return storage_type_; }
//
//        /**
//         * Get the binding location for the uniform buffer.
//         *
//         * Only valid when storage_type=StorageType::UNIFORM_BUFFER.
//         */
//        Location descriptor_set_location_get() const
//        {
//            return descriptor_set_location_;
//        }
//
//        /**
//         * Get the size needed to store the push constants.
//         */
//        uint32_t size_in_bytes() const { return size_in_bytes_; }
//
//        /**
//         * Find the push constant layout for the given location.
//         * Location = ShaderInput.location.
//         */
//        const PushConstant* find(int32_t location) const;
//
//        void debug_print() const;
//    };
//
//
//
//
//} // namespace Chandelier#pragma once
//
//#include "VkCommon.h"
//#include "Texture.h"
//#include "Descriptor.h"
//
//namespace Chandelier
//{
//    enum StorageType : uint8_t
//    {
//        None_Type = 0,
//        Push_Constants,
//        Uniform_Buffer,
//    };
//
//    struct Layout
//    {
//        static constexpr StorageType STORAGE_TYPE_DEFAULT  = StorageType::PUSH_CONSTANTS;
//        static constexpr StorageType STORAGE_TYPE_FALLBACK = StorageType::UNIFORM_BUFFER;
//
//        struct PushConstant
//        {
//            /* Used as lookup based on ShaderInput. */
//            int32_t location;
//
//            /** Offset in the push constant data (in bytes). */
//            uint32_t     offset;
//            shader::Type type;
//            int          array_size;
//        };
//
//    private:
//        std::vector<PushConstant> push_constants;
//        uint32_t             size_in_bytes_ = 0;
//        StorageType          storage_type_  = StorageType::None_Type;
//        /**
//         * Binding index in the descriptor set when the push constants use an uniform buffer.
//         */
//        Location descriptor_set_location_;
//
//    public:
//        /**
//         * Return the desired storage type that can fit the push constants of the given shader
//         * create info, matching the limits of the given device.
//         *
//         * Returns:
//         * - StorageType::NONE: No push constants are needed.
//         * - StorageType::PUSH_CONSTANTS: Regular vulkan push constants can be used.
//         * - StorageType::UNIFORM_BUFFER: The push constants don't fit in the limits of the
//         * given device. A uniform buffer should be used as a fallback method.
//         */
//        static StorageType determine_storage_type(const shader::ShaderCreateInfo& info,
//                                                  const VKDevice&                 device);
//
//        /**
//         * Initialize the push constants of the given shader create info with the
//         * binding location.
//         *
//         * interface: Uniform locations of the interface are used as lookup key.
//         * storage_type: The type of storage for push constants to use.
//         * location: When storage_type=StorageType::UNIFORM_BUFFER this contains
//         *    the location in the descriptor set where the uniform buffer can be
//         *    bound.
//         */
//        void init(const shader::ShaderCreateInfo& info,
//                  const VKShaderInterface&        interface,
//                  StorageType                     storage_type,
//                  Location       location);
//
//        /**
//         * Return the storage type that is used.
//         */
//        StorageType storage_type_get() const { return storage_type_; }
//
//        /**
//         * Get the binding location for the uniform buffer.
//         *
//         * Only valid when storage_type=StorageType::UNIFORM_BUFFER.
//         */
//        Location descriptor_set_location_get() const
//        {
//            return descriptor_set_location_;
//        }
//
//        /**
//         * Get the size needed to store the push constants.
//         */
//        uint32_t size_in_bytes() const { return size_in_bytes_; }
//
//        /**
//         * Find the push constant layout for the given location.
//         * Location = ShaderInput.location.
//         */
//        const PushConstant* find(int32_t location) const;
//
//        void debug_print() const;
//    };
//
//
//
//
//} // namespace Chandelier