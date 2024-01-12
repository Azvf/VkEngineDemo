#pragma once

#include "SimpleCamera.h"

class InputHandler {
public:
    InputHandler(GLFWwindow* window, Camera& camera) : m_window(window), m_camera(camera) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        glfwSetCursorPosCallback(window, 
            [] (GLFWwindow* window, double xpos, double ypos) {
                InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));

                float xoffset = xpos - inputHandler->m_lastCursorX;
                float yoffset = inputHandler->m_lastCursorY - ypos; // reversed since y-coordinates range from bottom to top

                inputHandler->m_lastCursorX = xpos;
                inputHandler->m_lastCursorY = ypos;

                inputHandler->m_camera.processMouseMovement(xoffset, yoffset, true);
            }
        );

        glfwSetKeyCallback(m_window, keyCallback);

        glfwSetWindowUserPointer(m_window, this);
    }

    void update() {
        // Get the current time and delta time since the last update
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - m_lastTime);
        m_lastTime = currentTime;

        // Update the camera orientation based on mouse input
        float deltaX = static_cast<float>(m_cursorX - m_lastCursorX);
        float deltaY = static_cast<float>(m_cursorY - m_lastCursorY);
        m_lastCursorX = m_cursorX;
        m_lastCursorY = m_cursorY;
        m_camera.rotate(deltaX, deltaY, deltaTime);

        // Update the camera position based on keyboard input
        glm::vec3 movement(0.0f);
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
            movement.z -= 1.0f;
        }
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
            movement.z += 1.0f;
        }
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
            movement.x -= 1.0f;
        }
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
            movement.x += 1.0f;
        }
        if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            movement.y += 1.0f;
        }
        if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            movement.y -= 1.0f;
        }
        m_camera.move(movement, deltaTime);
    }

private:
    static void cursorPositionCallback(GLFWwindow* m_window, double xpos, double ypos) {
        InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(m_window));
        inputHandler->m_cursorX = static_cast<float>(xpos);
        inputHandler->m_cursorY = static_cast<float>(ypos);
    }

    static void keyCallback(GLFWwindow* m_window, int key, int scancode, int action, int mods) {
        InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(m_window));
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_window, true);
        }
    }

    GLFWwindow* m_window;
    Camera& m_camera;
    double m_lastTime = glfwGetTime();
    float m_cursorX = 0.0f;
    float m_cursorY = 0.0f;
    float m_lastCursorX = 0.0f;
    float m_lastCursorY = 0.0f;
};


