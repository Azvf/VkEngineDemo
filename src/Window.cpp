#include "Window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

Window::Window(uint32_t width, uint32_t height, const char* title)
    : m_width(width), m_height(height), m_title(title)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_windowHandle = glfwCreateWindow(m_width, m_height, m_title.data(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_windowHandle, this);
    glfwSetFramebufferSizeCallback(m_windowHandle, framebufferResizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(m_windowHandle);
    glfwTerminate();
}

void* Window::getWindowHandle() const
{
    return m_windowHandle;
}

uint32_t Window::getWidth() const
{
    return m_width;
}

uint32_t Window::getHeight() const
{
    return m_height;
}

bool Window::isIconified() const
{
    return glfwGetWindowAttrib(m_windowHandle, GLFW_ICONIFIED);
}

void Window::resize(uint32_t width, uint32_t height)
{
    glfwSetWindowSize(m_windowHandle, width, height);
    int w, h;
    glfwGetWindowSize(m_windowHandle, &w, &h);
    m_width = w;
    m_height = h;
}

std::vector<std::pair<uint32_t, uint32_t>> Window::getSupportedResolutions()
{
    return std::vector<std::pair<uint32_t, uint32_t>>();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_windowHandle);
}

void Window::setTitle(const std::string& title)
{
    m_title = title;
}

void Window::PollEvents()
{
    glfwPollEvents();
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}