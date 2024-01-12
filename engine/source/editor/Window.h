#pragma once

#include <vector>
#include <string>

#include "IInputListener.h"
#include "UserInput.h"

struct GLFWwindow;

namespace Chandelier {
	class Window
	{
	private:
		friend void curserPosCallback(GLFWwindow* window, double xPos, double yPos);
		friend void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
		friend void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		friend void charCallback(GLFWwindow* window, unsigned int codepoint);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

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
		void addInputListener(sss::IInputListener* listener);
		void removeInputListener(sss::IInputListener* listener);
		sss::UserInput& getUserInput();

	private:
		GLFWwindow* m_windowHandle;
		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;
		bool framebufferResized = false;
		sss::UserInput m_userInput;
		std::vector<sss::IInputListener*> m_inputListeners;
	};
}