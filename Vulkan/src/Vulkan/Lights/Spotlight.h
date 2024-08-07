#pragma once

#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Vulkan/Components/VNameComponent.h"
#include "Vulkan/Components/VTransformComponent.h"

class Spotlight
{
public:

    Spotlight();
    Spotlight(VTransformComponent transform);
    Spotlight(VTransformComponent transform, VNameComponent name);

    const char* getNameStr() { return Name.GetName(); }
    VNameComponent getName() { return Name; }
    
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    glm::mat4 getViewProjectionMatrix();

public:
    VTransformComponent Transform;
    VNameComponent Name;
    
private:
    void updateProjectionMatrix();
private:
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
};
