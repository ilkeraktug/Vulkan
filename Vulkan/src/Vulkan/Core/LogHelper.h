#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class LogHelper
{
public:

    static std::string vec3_ToString(const glm::vec3& vector);
    static std::string vec4_ToString(const glm::vec4& vector);
    
    static std::string mat3_ToString(const glm::mat3& matrix);
    static std::string mat4_ToString(const glm::mat4& matrix);
};
