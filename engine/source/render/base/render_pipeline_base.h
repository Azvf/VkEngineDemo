#pragma once

#include <memory>

//namespace Chandelier {
//    struct RenderPassInitInfo {};
//
//    class VKContext;
//    class WindowUI;
//
//    struct RenderPassCommonInfo {
//        std::shared_ptr<VKContext> ctx;
//    };
//
//    class RenderPassBase {
//    public:
//        virtual void initialize(const RenderPassInitInfo* init_info) = 0;
//        virtual void postInitialize();
//        virtual void setCommonInfo(RenderPassCommonInfo common_info);
//        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);
//        virtual void initializeUIRenderBackend(WindowUI* window_ui);
//
//    protected:
//        RenderPassCommonInfo m_renderPassInfo;
//    };
//}