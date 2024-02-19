//#pragma once
//
//#include "VkCommon.h"
//
//namespace Chandelier
//{
//    // class VKContext;
//    // class Image;
//    // class Buffer;
//    // class Descriptor;
//    // class Sampler;
//    // 
//    // class MainRenderPass;
//    // class UIPass;
//
//    struct IBLResource
//    {
//        VkImage        _brdfLUT_texture_image;
//        VkImageView    _brdfLUT_texture_image_view;
//        VkSampler      _brdfLUT_texture_sampler;
//        VkDeviceMemory _brdfLUT_texture_image_allocation;
//
//        VkImage        _irradiance_texture_image;
//        VkImageView    _irradiance_texture_image_view;
//        VkSampler      _irradiance_texture_sampler;
//        VkDeviceMemory _irradiance_texture_image_allocation;
//
//        VkImage        _specular_texture_image;
//        VkImageView    _specular_texture_image_view;
//        VkSampler      _specular_texture_sampler;
//        VkDeviceMemory _specular_texture_image_allocation;
//    };
//
//    struct ColorGradingResource
//    {
//        VkImage        _color_grading_LUT_texture_image;
//        VkImageView    _color_grading_LUT_texture_image_view;
//        VkDeviceMemory _color_grading_LUT_texture_image_allocation;
//    };
//
//    struct StorageBuffer
//    {
//        // limits
//        uint32_t _min_uniform_buffer_offset_alignment {256};
//        uint32_t _min_storage_buffer_offset_alignment {256};
//        uint32_t _max_storage_buffer_range {1 << 27};
//        uint32_t _non_coherent_atom_size {256};
//    };
//
//    struct GlobalRenderResource
//    {
//        IBLResource          _ibl_resource;
//        ColorGradingResource _color_grading_resource;
//        StorageBuffer        _storage_buffer;
//    };
//
//    // struct RenderResource
//    // {
//    // public:
//    //     std::shared_ptr<VKContext> m_context;
//    // 
//    //     // images
//    //     std::unique_ptr<Image> m_depthStencilImage[MAX_FRAMES_IN_FLIGHT];
//    //     std::unique_ptr<Image> m_colorImage[MAX_FRAMES_IN_FLIGHT];
//    //     std::unique_ptr<Image> m_diffuseImage[MAX_FRAMES_IN_FLIGHT];
//    //     std::unique_ptr<Image> m_tonemappedImage[MAX_FRAMES_IN_FLIGHT];
//    // 
//    //     // framebuffers
//    //     VkFramebuffer              m_mainFramebuffers[MAX_FRAMES_IN_FLIGHT];
//    //     VkFramebuffer              m_shadowFramebuffers[MAX_FRAMES_IN_FLIGHT];
//    //     std::vector<VkFramebuffer> m_guiFramebuffers;
//    // 
//    //     // const buffers
//    //     std::unique_ptr<Buffer> m_constantBuffer[MAX_FRAMES_IN_FLIGHT];
//    // 
//    //     // depth image view
//    //     VkImageView m_depthImageView[MAX_FRAMES_IN_FLIGHT];
//    // 
//    //     // render passes
//    //     std::unique_ptr<MainRenderPass> m_mainPass;
//    //     std::unique_ptr<UIPass>         m_uiPass;
//    // 
//    //     // descriptors
//    //     std::unique_ptr<Descriptor> m_texDescriptor;
//    //     std::unique_ptr<Descriptor> m_lightingDescriptor;
//    //     std::unique_ptr<Descriptor> m_postProcDescriptor;
//    // 
//    //     // samplers
//    //     std::unique_ptr<Sampler> m_shadowSampler;
//    //     std::unique_ptr<Sampler> m_linearSamplerClamp;
//    //     std::unique_ptr<Sampler> m_linearSamplerRepeat;
//    //     std::unique_ptr<Sampler> m_pointSamplerClamp;
//    //     std::unique_ptr<Sampler> m_pointSamplerRepeat;
//    // 
//    // public:
//    //     void Resize(int32_t width, int32_t height);
//    // };
//
//} // namespace Chandelier
