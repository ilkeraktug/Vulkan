#include "pch.h"
#include "Spotlight.h"

#include <glm/ext/matrix_transform.hpp>

#include "Vulkan/Global/ObjectManager.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"

Spotlight::Spotlight()
    : Transform()
{
    Name.SetName("Spotlight_");
    char append[1];
    std::printf(append, "%llu", ObjectManager::GObjectIndex++);
    Name.Append(append);
    
    updateProjectionMatrix();
}

Spotlight::Spotlight(VTransformComponent transform)
    : Transform(transform)
{
    Name.SetName("Spotlight_");
    char append[1];
    std::printf(append, "%llu", ObjectManager::GObjectIndex++);
    Name.Append(append);

    updateProjectionMatrix();
}

Spotlight::Spotlight(VTransformComponent transform, VNameComponent name)
    : Transform(transform), Name(name)
{
    updateProjectionMatrix();
}

glm::mat4 Spotlight::getViewMatrix()
{
    return Transform.getModelMatrix();
}

glm::mat4 Spotlight::getProjectionMatrix()
{
    return m_ProjectionMatrix;
}

glm::mat4 Spotlight::getViewProjectionMatrix()
{
    return m_ProjectionMatrix * m_ViewMatrix;
}

void Spotlight::updateProjectionMatrix()
{
    m_ProjectionMatrix = glm::perspective(45.0f, 1.0f, 1.0f, 100.0f);
}
