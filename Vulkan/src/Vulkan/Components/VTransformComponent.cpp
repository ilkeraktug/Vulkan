#include "pch.h"
#include "VTransformComponent.h"

#include <glm/ext/matrix_Transform.hpp>

VTransformComponent::VTransformComponent()
    : m_Position(0.0f), m_Rotation(0.0f)
{
}

void VTransformComponent::setPosition(glm::vec3 position)
{
    m_Position = position;
}

void VTransformComponent::setRotation(glm::vec3 rotation)
{
    m_Rotation = rotation;
}

glm::vec3 VTransformComponent::getPosition()
{
    return m_Position;
}

glm::vec3 VTransformComponent::getRotation()
{
    return m_Rotation;
}

glm::mat4 VTransformComponent::getModelMatrix()
{
    glm::mat4 position = glm::translate(glm::mat4(1.0f), m_Position);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    return position * rotation;
}
