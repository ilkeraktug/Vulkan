#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoords;

layout(binding = 2) uniform sampler2D u_Texture;

void main() 
{
   outColor = texture(u_Texture, v_TexCoords);
   //outColor = vec4(1.0f);
}