#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vulkan/Renderer/VulkanUniformBuffer.h"

class Transform
{
public:
    Transform(glm::vec3 Position = Transform::ZeroVector, glm::vec3 Rotation = Transform::ZeroVector, glm::vec3 scale = Transform::OneVector);

    void Translate(const glm::vec3& position);
    void SetPosition(const glm::vec3& position);

    void Rotate(float angle, const glm::vec3 axis);
    void SetRotation(const glm::vec3& rotation);

    void SetScale(const glm::vec3& scale);

    const glm::mat4& GetModelMatrix() const { return m_ModelMatrix; }
    VkTransformMatrixKHR GetAsVkTransformMatrixKHR() const;

    operator VkTransformMatrixKHR() const { return GetAsVkTransformMatrixKHR(); }
private:
    void UpdateModelMatrix();

    void SetPositionNoUpdate(const glm::vec3& position);
    void SetRotationNoUpdate(const glm::vec3& rotation);
    void SetScaleNoUpdate(const glm::vec3& scale);
    
private:
    glm::mat4 m_PositionMatrix;
    glm::mat4 m_RotationMatrix;
    glm::mat4 m_ScaleMatrix;
    
    glm::mat4 m_ModelMatrix;

public:

    static glm::vec3 WorldForward;
    static glm::vec3 WorldRight;
    static glm::vec3 WorldUp;

    static glm::vec3 ZeroVector;
    static glm::vec3 OneVector;

    static glm::mat4 Identity4x4Matrix;

    static Transform Identity;
};
