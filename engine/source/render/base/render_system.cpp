#include "render_system.h"

#include <thread>
#include <chrono>

#include "UI/window_system.h"
#include "editor/camera.h"
#include "render/passes/main_pass.h"
#include "render/passes/skybox_pass.h"
#include "render/passes/ui_pass.h"
#include "render/passes/shadowmap_pass.h"
#include "render/precompute/brdf_lut.h"
#include "render/precompute/cubemap_prefilter.h"
#include "render/precompute/irradiance_convolution_pass.h"

#include "VkContext.h"
#include "Texture.h"

namespace Chandelier
{
    RenderSystem::~RenderSystem() { UnInit(); }

    void RenderSystem::SetupUnifromBuffers()
    {
        { // main pass ubo
            m_main_pass_uniform_buffer                              = std::make_shared<MainPassUniformBuffer>();
            m_main_pass_uniform_buffer->config.rotating             = true;
            // m_main_pass_uniform_buffer->config.anti_aliasing        = Enable_MSAA;
            m_main_pass_uniform_buffer->config.tone_mapping         = true;
            m_main_pass_uniform_buffer->config.use_gamma_correction = true;
        }
        
        // set initial aa flag
        m_anti_aliasing = (eAntiAliasing)m_main_pass_uniform_buffer->config.anti_aliasing;

        {   // skybox pass ubo
            m_skybox_pass_uniform_buffer                             = std::make_shared<SkyboxPassUniformBuffer>();
            m_skybox_pass_uniform_buffer->inv_model_view_projection  = glm::mat4(1.0);
            m_skybox_pass_uniform_buffer->show_skybox_index          = 0;
            m_skybox_pass_uniform_buffer->skybox_prefilter_mip_level = 0.0;
        }

        {   // shadowmap pass ubo
            m_shadowmap_pass_uniform_buffer = std::make_shared<ShadowmapPassUniformBuffer>();
            m_shadowmap_pass_uniform_buffer->view = glm::mat4(1.0);
            m_shadowmap_pass_uniform_buffer->projection = glm::mat4(1.0);
        }
    }

    void RenderSystem::SetupRenderPasses()
    {
        auto main_pass_init_info                       = std::make_shared<MainRenderPassInitInfo>();
        main_pass_init_info->render_context.vk_context = m_context;
        main_pass_init_info->memory_uniform_buffer     = m_main_pass_uniform_buffer;
        main_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        main_pass_init_info->height                    = m_window_system->GetWindowSize().y;
        main_pass_init_info->render_resources          = m_render_resources;
        m_main_pass                                    = std::make_shared<MainRenderPass>();
        m_main_pass->Initialize(main_pass_init_info);

        auto skybox_pass_init_info                       = std::make_shared<SkyboxPassInitInfo>();
        skybox_pass_init_info->render_context.vk_context = m_context;
        skybox_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        skybox_pass_init_info->main_pass_uniform_buffer  = m_main_pass_uniform_buffer;
        skybox_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        skybox_pass_init_info->height                    = m_window_system->GetWindowSize().y;
        skybox_pass_init_info->render_resources          = m_render_resources;
        m_skybox_pass = std::make_shared<SkyboxPass>();
        m_skybox_pass->Initialize(skybox_pass_init_info);

        auto ui_pass_init_info                        = std::make_shared<UIPassInitInfo>();
        ui_pass_init_info->render_context.vk_context  = m_context;
        ui_pass_init_info->window_system              = m_window_system;
        ui_pass_init_info->main_pass_uniform_buffer   = m_main_pass_uniform_buffer;
        ui_pass_init_info->skybox_pass_uniform_buffer = m_skybox_pass_uniform_buffer;
        ui_pass_init_info->render_pass                = m_main_pass->GetRenderPass();
        ui_pass_init_info->width                      = m_window_system->GetWindowSize().x;
        ui_pass_init_info->height                     = m_window_system->GetWindowSize().y;
        m_ui_pass = std::make_shared<UIPass>();
        m_ui_pass->Initialize(ui_pass_init_info);

        auto shadowmap_pass_init_info                       = std::make_shared<ShadowmapPassInitInfo>();
        shadowmap_pass_init_info->render_context.vk_context = m_context;
        shadowmap_pass_init_info->render_pass               = m_main_pass->GetRenderPass();
        shadowmap_pass_init_info->main_pass_uniform_buffer  = m_main_pass_uniform_buffer;
        shadowmap_pass_init_info->width                     = m_window_system->GetWindowSize().x;
        shadowmap_pass_init_info->height                    = m_window_system->GetWindowSize().y;
        shadowmap_pass_init_info->render_resources          = m_render_resources;
        m_shadowmap_pass = std::make_shared<ShadowmapPass>();
        m_shadowmap_pass->Initialize(shadowmap_pass_init_info);

        m_main_pass->SetShadowMapPass(m_shadowmap_pass);
    }

    void RenderSystem::Initialize(std::shared_ptr<WindowSystem> window_system)
    {
        m_window_system = window_system;
        m_context       = std::make_shared<VKContext>();
        m_context->Initialize(m_window_system);

        LoadAssets();
        
        SetupUnifromBuffers();

        SetupRenderPasses();

        m_arcball_camera = std::make_shared<sss::ArcBallCamera>(glm::vec3(0.0f, 0.0f, 0.0f), 4.0f);
        // m_arcball_camera->update(glm::vec2(0.0, 45.0), 0.0);

        SetupLightingSet();

        PreRenderSetup();
    }

    void RenderSystem::UnInit() {}

    void RenderSystem::SetupLightingSet()
    {
        auto& lights = m_main_pass_uniform_buffer->lights;

        lights.point_light_num = 0;
        for (int i = 0; i < lights.point_light_num; i++)
        {
            lights.point_lights[i].color     = Vector3(1.0, 1.0, 1.0);
            lights.point_lights[i].intensity = 100.0;
            lights.point_lights[i].radius    = 10.0;
        }

        float base_distance = 3.0;

        // lights.point_lights[0].color    = Vector3(0.08, 0.29, 0.47);
        lights.point_lights[0].position = Vector3(base_distance, base_distance, base_distance);

        lights.point_lights[1].position = Vector3(-base_distance, -base_distance, -base_distance);
        lights.point_lights[2].position = Vector3(base_distance, -base_distance, base_distance);
        lights.point_lights[3].position = Vector3(-base_distance, -base_distance, base_distance);
    }

    void RenderSystem::LoadAssets() { 
        auto& command_manager = m_context->GetCommandManager();
        m_render_resources = std::make_shared<RenderResources>();
        {
            std::string asset_dir = "G:/Visual Studio Projects/VkEngineDemo/engine/assets/base models/";
            
            // mesh
            m_render_resources->model_mesh_vec.push_back(LoadStaticMesh(m_context, asset_dir + "sphere.obj"));
            
            // textures
            asset_dir = "G:/Visual Studio Projects/VkEngineDemo/engine/assets/dragon-scales/";
            m_render_resources->model_tex_vec.push_back(LoadTexture(
                m_context, asset_dir + "dragon-scales_albedo.png", SRGB_Color_Space));
            m_render_resources->model_tex_vec.push_back(LoadTexture(
                m_context, asset_dir + "dragon-scales_normal-ogl.png", Linear_Color_Space));
            m_render_resources->model_tex_vec.push_back(LoadTexture(
                m_context, asset_dir + "dragon-scales_ao.png", Linear_Color_Space));
            m_render_resources->model_tex_vec.push_back(LoadTexture(
                m_context, asset_dir + "dragon-scales_metallic.png", Linear_Color_Space));
            m_render_resources->model_tex_vec.push_back(LoadTexture(
                m_context, asset_dir + "dragon-scales_roughness.png", Linear_Color_Space));

            // brdf lut
            asset_dir                    = "G:/Visual Studio Projects/VkEngineDemo/build/engine/source/";
            // m_render_resources->brdf_lut = LoadTexture(m_context, asset_dir + "brdf_lut.png", Linear_Color_Space);
            m_render_resources->brdf_lut = LoadTextureHDR(m_context, asset_dir + "brdf_schilk.hdr", 4);

            // screen mesh
            m_render_resources->screen_mesh = LoadDefaultMesh(m_context, Screen_Mesh);

            // cube mesh
            m_render_resources->cube_mesh = LoadDefaultMesh(m_context, Cube_Mesh);

            {
                // skybox cubemap
                std::array<std::shared_ptr<Texture>, 6> skybox_faces;
                skybox_faces[0] = LoadTextureHDR(
                    m_context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_X+.hdr", 4);
                skybox_faces[1] = LoadTextureHDR(
                    m_context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_X-.hdr", 4);
                skybox_faces[2] = LoadTextureHDR(
                    m_context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Z+.hdr", 4);
                skybox_faces[3] = LoadTextureHDR(
                    m_context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Z-.hdr", 4);
                skybox_faces[4] = LoadTextureHDR(
                    m_context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Y+.hdr", 4);
                skybox_faces[5] = LoadTextureHDR(
                    m_context, "G:/Visual Studio Projects/VkEngineDemo/engine/assets/skybox/skybox_specular_Y-.hdr", 4);
                m_render_resources->skybox_cubemap = LoadSkybox(m_context, skybox_faces, 4);
            }
            
            {
                // skybox prefilter cubemap
                m_render_resources->skybox_prefilter_cubemap = std::make_shared<Texture>();

                m_render_resources->skybox_prefilter_cubemap->InitCubeMap(
                    m_context,
                    CUBEMAP_PREFILTER_BASE_WIDTH,
                    CUBEMAP_PREFILTER_MIP_LEVEL,
                    PREFILTER_ATTACHMENT_FORMAT,
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

                m_render_resources->skybox_prefilter_cubemap->TransferLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                                             0,
                                                                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                                             VK_ACCESS_TRANSFER_WRITE_BIT);

                m_context->GenerateMipMaps(m_render_resources->skybox_prefilter_cubemap.get(),
                                           CUBEMAP_PREFILTER_MIP_LEVEL);
            }

            {
                // skybox irradiance cubemap
                m_render_resources->skybox_irradiance_cubemap = std::make_shared<Texture>();
            
                m_render_resources->skybox_irradiance_cubemap->InitCubeMap(
                    m_context,
                    IRRADIANCE_MAP_WIDTH,
                    IRRADIANCE_MAP_HEIGHT,
                    IRRADIANCE_ATTACHMENT_FORMAT,
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
            
                m_render_resources->skybox_irradiance_cubemap->TransferLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                                             0,
                                                                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                                             VK_ACCESS_TRANSFER_WRITE_BIT);
                command_manager.Submit();
            }
            
        }
    }

    void RenderSystem::PreRenderSetup() { m_main_pass->PreDrawSetup(); }

    void RenderSystem::Render() { 
        auto& swapchain = m_context->GetSwapchain();
        auto  extent    = swapchain.getExtent();
        float ar        = extent.width / float(extent.height);

        if (m_main_pass_uniform_buffer->config.rotating)
        {
            // m_camera->rotate(glm::vec3(0.f, 1.f, 0.f));
            m_arcball_camera->update(glm::vec2(1.0, 0.0), 0.0);
            // m_arcball_camera->update(glm::vec2(1.0, 0.0), 0.0);
        }
        
        constexpr float fovy = glm::radians(40.0f);

        const glm::mat4 vulkanCorrection = {
            {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f, 1.0f}};

        const glm::mat4 viewMatrix = m_arcball_camera->getViewMatrix();
        const glm::mat4 projectionMatrix = vulkanCorrection * glm::perspective(fovy, ar, 1.0f, 50.0f);
        const glm::mat4 viewProjection =
            vulkanCorrection * glm::perspective(fovy, ar, 0.01f, 50.0f) * viewMatrix;

        m_skybox_pass_uniform_buffer->inv_model_view_projection = glm::inverse(viewProjection);
        m_skybox_pass->UpdateUniformBuffer(*m_skybox_pass_uniform_buffer);

        m_shadowmap_pass_uniform_buffer->view       = viewMatrix;
        m_shadowmap_pass_uniform_buffer->projection = projectionMatrix;
        m_shadowmap_pass->UpdateUniformBuffer(*m_shadowmap_pass_uniform_buffer);

        m_main_pass_uniform_buffer->camera.position = glm::vec4(m_arcball_camera->getPosition(), 1.0);
        m_main_pass_uniform_buffer->camera.view       = viewMatrix;
        m_main_pass_uniform_buffer->camera.projection = projectionMatrix;
        m_main_pass->UpdateUniformBuffer(*m_main_pass_uniform_buffer);

        m_need_recreate |= (m_anti_aliasing != m_main_pass_uniform_buffer->config.anti_aliasing);
        if (m_need_recreate)
        {
            m_main_pass->Recreate();
            m_skybox_pass->Recreate();
            m_ui_pass->Recreate();
            m_shadowmap_pass->Recreate();

            m_anti_aliasing = (eAntiAliasing)m_main_pass_uniform_buffer->config.anti_aliasing;
            m_need_recreate = false;
        }

        swapchain.AcquireImage(m_need_recreate);

        if (m_need_recreate)
        {
            return;
        }

        std::vector<std::shared_ptr<RenderPass>> subpasses {m_skybox_pass, m_ui_pass};
        
        m_main_pass->ForwardDraw(subpasses);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        swapchain.SwapBuffer(m_need_recreate);

        if (m_need_recreate)
        {
            return;
        }
    }

} // namespace Chandelier
