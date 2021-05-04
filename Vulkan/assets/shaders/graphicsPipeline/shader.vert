#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 0) out vec4 v_Color;

layout(binding = 0) uniform UBOMatrices
{
    mat4 Model;
} uboModel;

layout(binding = 1) uniform UBO
{
    mat4 ViewProjection;
} uboViewProjection;

void main() 
{
    gl_Position = uboViewProjection.ViewProjection * uboModel.Model * a_Position;
    v_Color = a_Color;

}