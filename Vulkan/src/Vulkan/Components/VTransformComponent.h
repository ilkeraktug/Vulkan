#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/detail/type_vec3.hpp>

class VTransformComponent
{
public:

    VTransformComponent();

    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);

    glm::vec3 getPosition();
    glm::vec3 getRotation();
    glm::mat4 getModelMatrix();
    
private:
    glm::vec3 m_Position;
    glm::vec3 m_Rotation;
};
