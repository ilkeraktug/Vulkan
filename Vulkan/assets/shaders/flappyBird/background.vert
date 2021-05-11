#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 0) out vec2 v_TexCoords;

layout(binding = 0) uniform UBOMatrices
{
    mat4 Model;
} uboModel;

void main() 
{
    gl_Position = uboModel.Model * a_Position;

    v_TexCoords = a_TexCoords;

}