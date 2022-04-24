#include "pch.h"
#include "PipeObject.h"

PipeObject::PipeObject(const std::string& texturePath, VulkanCore* core)
	:QuadObj(core)
{
	Texture.reset(new VulkanTexture2D(texturePath, core));
}
