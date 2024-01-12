#include "Window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "VkUtil.h"
#include "UserInput.h"

template<typename T>
void remove(std::vector<T>& v, const T& item) {
	v.erase(std::remove(v.begin(), v.end(), item), v.end());
}
namespace Chandelier {
	Window::Window(uint32_t width, uint32_t height, const char* title)
		: m_width(width), m_height(height), m_title(title)
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_windowHandle = glfwCreateWindow(m_width, m_height, m_title.data(), nullptr, nullptr);

		addInputListener(&m_userInput);

		glfwSetCursorPosCallback(m_windowHandle, curserPosCallback);
		glfwSetScrollCallback(m_windowHandle, scrollCallback);
		glfwSetMouseButtonCallback(m_windowHandle, mouseButtonCallback);
		glfwSetKeyCallback(m_windowHandle, keyCallback);
		glfwSetCharCallback(m_windowHandle, charCallback);

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

	void Window::addInputListener(sss::IInputListener* listener) {
		m_inputListeners.push_back(listener);
	}

	void Window::removeInputListener(sss::IInputListener* listener) {
		remove(m_inputListeners, listener);
	}

	sss::UserInput& Window::getUserInput()
	{
		return m_userInput;
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}


	// callback functions
	void curserPosCallback(GLFWwindow* window, double xPos, double yPos)
	{
		Window* windowFramework = static_cast<Window*>(glfwGetWindowUserPointer(window));
		for (sss::IInputListener* listener : windowFramework->m_inputListeners)
		{
			listener->onMouseMove(xPos, yPos);
		}
	}

	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		Window* windowFramework = static_cast<Window*>(glfwGetWindowUserPointer(window));
		for (sss::IInputListener* listener : windowFramework->m_inputListeners)
		{
			listener->onMouseScroll(xOffset, yOffset);
		}
	}

	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		Window* windowFramework = static_cast<Window*>(glfwGetWindowUserPointer(window));
		for (sss::IInputListener* listener : windowFramework->m_inputListeners)
		{
			listener->onMouseButton(static_cast<InputMouse>(button), static_cast<InputAction>(action));
		}
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Window* windowFramework = static_cast<Window*>(glfwGetWindowUserPointer(window));
		for (sss::IInputListener* listener : windowFramework->m_inputListeners)
		{
			listener->onKey(static_cast<InputKey>(key), static_cast<InputAction>(action));
			printf("key: %c\n", key);
		}
	}

	void charCallback(GLFWwindow* window, unsigned int codepoint)
	{
		Window* windowFramework = static_cast<Window*>(glfwGetWindowUserPointer(window));
		for (sss::IInputListener* listener : windowFramework->m_inputListeners)
		{
			listener->onChar(codepoint);
		}
	}
}