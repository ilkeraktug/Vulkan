#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D u_Texture;

layout(location = 0) in vec3 v_FragColor;
layout(location = 1) in vec2 v_TexCoords;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(u_Texture, v_TexCoords);
}