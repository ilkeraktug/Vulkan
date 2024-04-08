#include "pch.h"
#include "Transform.h"


glm::vec3 Transform::WorldForward   = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 Transform::WorldRight     = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 Transform::WorldUp        = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 Transform::ZeroVector = glm::vec3(0.0f);
glm::vec3 Transform::OneVector  = glm::vec3(1.0f);

glm::mat4 Transform::Identity4x4Matrix  = glm::mat4(1.0f);

Transform Transform::Identity = Transform();


Transform::Transform(glm::vec3 Position, glm::vec3 Rotation, glm::vec3 scale)
{
    SetPositionNoUpdate(Position);
    SetRotationNoUpdate(Rotation);
    SetScaleNoUpdate(scale);

    UpdateModelMatrix();
}

void Transform::Translate(const glm::vec3& position)
{
    m_PositionMatrix = glm::translate(m_PositionMatrix, position);

    UpdateModelMatrix();
}

void Transform::SetPosition(const glm::vec3& position)
{
    SetPositionNoUpdate(position);

    UpdateModelMatrix();
}

void Transform::Rotate(float angle, const glm::vec3 axis)
{
    m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(angle), axis);

    UpdateModelMatrix();
}

void Transform::SetRotation(const glm::vec3& rotation)
{
    SetRotationNoUpdate(rotation);

    UpdateModelMatrix();
}

void Transform::SetScale(const glm::vec3& scale)
{
    SetScaleNoUpdate(scale);

    UpdateModelMatrix();
}

VkTransformMatrixKHR Transform::GetAsVkTransformMatrixKHR() const
{
    VkTransformMatrixKHR transformMatrix;

    memcpy(&transformMatrix, &m_ModelMatrix, sizeof(VkTransformMatrixKHR));

    return transformMatrix;
}

void Transform::UpdateModelMatrix()
{
    m_ModelMatrix = glm::transpose(m_PositionMatrix * m_RotationMatrix * m_ScaleMatrix);
}

void Transform::SetPositionNoUpdate(const glm::vec3& position)
{
    m_PositionMatrix = glm::translate(Identity4x4Matrix, position);
}

void Transform::SetRotationNoUpdate(const glm::vec3& rotation)
{
    m_RotationMatrix =  glm::rotate(Identity4x4Matrix, glm::radians(rotation.x), WorldForward)  *
                        glm::rotate(Identity4x4Matrix, glm::radians(rotation.y), WorldRight)    *
                        glm::rotate(Identity4x4Matrix, glm::radians(rotation.z), WorldUp);
}

void Transform::SetScaleNoUpdate(const glm::vec3& scale)
{
    m_ScaleMatrix = glm::scale(Identity4x4Matrix, scale);
}
