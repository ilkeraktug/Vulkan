#include "pch.h"
#include "LogHelper.h"

#include <glm/gtc/matrix_transform.hpp>


std::string LogHelper::vec3_ToString(const glm::vec3& vector)
{
    std::stringstream Log;

    Log << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";

    return Log.str();
}

std::string LogHelper::vec4_ToString(const glm::vec4& vector)
{
    std::stringstream Log;

    Log << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";

    return Log.str();
}

std::string LogHelper::mat3_ToString(const glm::mat3& matrix)
{
    std::stringstream Log;
    
    Log << vec3_ToString(matrix[0]) << vec3_ToString(matrix[1]) << vec3_ToString(matrix[2]);

    return Log.str();
}

std::string LogHelper::mat4_ToString(const glm::mat4& matrix)
{
    std::stringstream Log;
    
    Log << "\n\t" << vec4_ToString(matrix[0]) << "\n\t" << vec4_ToString(matrix[1]) << "\n\t" << vec4_ToString(matrix[2]) << "\n\t" << vec4_ToString(matrix[3]);

    return Log.str();
}
