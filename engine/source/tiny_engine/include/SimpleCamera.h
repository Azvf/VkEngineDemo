#pragma once

#define WIN32_LEAN_AND_MEAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#undef near
#undef far
#undef min
#undef max

class Camera {
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f,
        float fov = 45.0f, float aspectRatio = 16.0f / 9.0f,
        float nearPlane = 0.1f, float farPlane = 100.0f)
        : m_position(position), m_up(up), m_yaw(yaw), m_pitch(pitch),
        m_fov(fov), m_aspectRatio(aspectRatio),
        m_nearPlane(nearPlane), m_farPlane(farPlane)
    {
        updateVectors();
    }

    void move(glm::vec3 offset, float deltaTime) {
        m_position += m_speed * deltaTime * glm::normalize(glm::vec3(offset.x, 0.0f, offset.z));
        m_position += m_speed * deltaTime * glm::normalize(glm::vec3(0.0f, offset.y, 0.0f));
    }

    void rotate(float xoffset, float yoffset, float deltaTime) {
        m_yaw += xoffset * m_sensitivity * deltaTime;
        m_pitch += yoffset * m_sensitivity * deltaTime;

        if (m_pitch > 89.0f) {
            m_pitch = 89.0f;
        }
        if (m_pitch < -89.0f) {
            m_pitch = -89.0f;
        }

        updateVectors();
    }

    void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
    {
        xoffset *= m_sensitivity;
        yoffset *= m_sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        if (constrainPitch) {
            if (m_pitch > 89.0f) {
                m_pitch = 89.0f;
            }
            if (m_pitch < -89.0f) {
                m_pitch = -89.0f;
            }
        }

        updateVectors();
    }

public:
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
    }

    glm::mat4 getViewProjectionMatrix() const {
        glm::mat4 viewMatrix = getViewMatrix();
        glm::mat4 projectionMatrix = getProjectionMatrix();
        return projectionMatrix * viewMatrix;
    }

    glm::mat4 getModelViewProjectionMatrix(const glm::mat4& modelMatrix) const {
        glm::mat4 viewProjectionMatrix = getViewProjectionMatrix();
        return viewProjectionMatrix * modelMatrix;
    }

private:
    void updateVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;

    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_speed = 5.0f;
    float m_sensitivity = 0.1f;
};
