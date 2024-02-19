#include "window_system.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "runtime/core/base/exception.h"

#include "VkContext.h"

namespace Chandelier
{
    WindowSystem::WindowSystem(Vector2i window_size, std::string_view title) :
        m_window_size(window_size), m_title(title)
    {}

    WindowSystem::~WindowSystem() { UnInit(); }

    void WindowSystem::Initialize() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_handle = glfwCreateWindow(m_window_size.x, m_window_size.y, m_title.data(), nullptr, nullptr);
        assert(m_handle);
        
        glfwSetWindowUserPointer(m_handle, this);
        glfwSetFramebufferSizeCallback(m_handle, WindowResizeCallback);
    }

    void WindowSystem::UnInit()
    {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
    }

    VkSurfaceKHR WindowSystem::CreateSurface(std::shared_ptr<VKContext> context)
    {
        VkSurfaceKHR surface;
        VULKAN_API_CALL(glfwCreateWindowSurface(context->getInstance(), m_handle, nullptr, &surface));
        return surface;
    }

    Vector2i WindowSystem::GetWindowSize() { return m_window_size; }

    Vector2i WindowSystem::GetFramebufferSize()
    {
        glm::ivec2 temp;
        Vector2i   fb_size {};
        glfwGetFramebufferSize(m_handle, &fb_size.x, &fb_size.y);
        return fb_size;
    }

    void WindowSystem::WaitEvents() { glfwWaitEvents(); }

    bool WindowSystem::ShouldClose() { return glfwWindowShouldClose(m_handle); }

    bool WindowSystem::IsIconified() { return glfwGetWindowAttrib(m_handle, GLFW_ICONIFIED); }

    void WindowSystem::PollEvents() { glfwPollEvents(); }

    void WindowSystem::WindowResizeCallback(GLFWwindow* window_handle, int width, int height)
    {
        auto window_system = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window_handle));
        // todo: impl
    }

} // namespace Chandelier
