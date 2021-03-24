#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBuffer
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
} u_Transformation;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 0) out vec3 v_FragColor;
layout(location = 1) out vec2 v_TexCoords;

void main() {
    gl_Position = u_Transformation.Projection * u_Transformation.View * u_Transformation.Model * vec4(a_Position, 1.0f);
    v_FragColor = a_Color;
    v_TexCoords = a_TexCoords;
}