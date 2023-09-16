#version 450

layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	mat4 lightSpace;
	vec4 lightPos;
	float zNear;
	float zFar;
} ubo;

float LinearizeDepth(float depth)
{
  float n = ubo.zNear;
  float f = ubo.zFar;
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main() 
{
	float depth = texture(samplerColor, inUV).r;
	//outFragColor = vec4(inUV, 0.0f, 1.0f);
	//outFragColor = vec4(inUV, 0.0f, 1.0f);
	outFragColor = vec4(1.0f - vec3(LinearizeDepth(depth)), 1.0);
	//outFragColor = vec4((LinearizeDepth(depth)), vec2(0.0f), 1.0);
}