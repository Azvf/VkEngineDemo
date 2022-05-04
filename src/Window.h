#pragma once

#include <vector>
#include <string>

struct GLFWwindow;

class Window
{
public:
	explicit Window(uint32_t width, uint32_t height, const char* title);
	~Window();
	void* getWindowHandle() const;
	uint32_t getWidth() const;
	uint32_t getHeight() const;
	bool isIconified() const;
	void resize(uint32_t width, uint32_t height);
	std::vector<std::pair<uint32_t, uint32_t>> getSupportedResolutions();
	bool shouldClose() const;
	void setTitle(const std::string& title);
	void PollEvents();
	static void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	GLFWwindow* m_windowHandle;
	uint32_t m_width;
	uint32_t m_height;
	std::string m_title;
	bool framebufferResized = false;
};