#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vulkan/Renderer/VulkanUniformBuffer.h"
#include "Vulkan/Core/LogHelper.h"

class Camera
{
public:

    virtual ~Camera() = default;
    
    void setPosition(const glm::vec3& position)
    {
        m_Position = position;
        RecalculateViewMatrix();
    }

    void addPosition(const glm::vec3& deltaPosition)
    {
        m_Position += deltaPosition;
        RecalculateViewMatrix();
    }

    void setRotation(const glm::vec3& rotation)
    {
        m_Rotation = rotation;
        RecalculateViewMatrix();
    }
    void addRotation(const glm::vec3& deltaRotation)
    {
        m_Rotation += deltaRotation;
        RecalculateViewMatrix();
    }
    
    const glm::vec3& getForwardVector() const { return m_FrontVector; }
    const glm::vec3& getRightVector() const { return m_RightVector; }
    const glm::vec3& getUpVector() const { return m_UpVector; }

    const glm::vec3& getPosition() const { return m_Position; }
    const glm::vec3& getRotation() const { return m_Rotation; }

    const glm::mat4& getViewMatrix() const { return m_ViewMatrix; }
    glm::mat4& getViewMatrixRef() { return m_ViewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return m_ProjectionMatrix; }
    const glm::mat4& getViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    VkDescriptorSet descriptorSet;

public:
    std::unique_ptr<VulkanUniformBuffer> MatricesBuffer;

protected:
    void RecalculateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        
        rotM = glm::rotate(rotM, glm::radians(m_Rotation.x * -1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = m_Position;
        translation.y *= -1.0f;

        glm::mat4 transM = glm::translate(glm::mat4(1.0f), translation);

        m_ViewMatrix = rotM * transM;
        
        m_ViewPosition = glm::vec4(m_Position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

        m_FrontVector.x = -cos(glm::radians(m_Rotation.x)) * sin(glm::radians(m_Rotation.y));
        m_FrontVector.y = sin(glm::radians(m_Rotation.x));
        m_FrontVector.z = cos(glm::radians(m_Rotation.x)) * cos(glm::radians(m_Rotation.y));
        m_FrontVector = glm::normalize(m_FrontVector);

        m_RightVector = glm::cross(m_FrontVector, glm::vec3(0.0f, 1.0f, 0.0f));
        m_RightVector = glm::normalize(m_RightVector);

        m_UpVector = glm::normalize(glm::cross(m_FrontVector, m_RightVector));
        
        updateUniformBuffers();
    }
    
    void updateUniformBuffers()
    {
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        MatricesBuffer->copyData(&m_ViewProjectionMatrix, sizeof(glm::mat4));
    }

protected:
    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);;
    glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

    glm::vec3 m_Position = {0.0f, 0.0f, -0.5f};
    glm::vec3 m_ViewPosition = {0.0f, 0.0f, -0.5f};
    glm::vec3 m_Rotation = glm::vec3(0.0f);

    glm::vec3 m_FrontVector = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 m_RightVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_UpVector    = glm::vec3(0.0f, 0.0f, 1.0f);

private:
};
