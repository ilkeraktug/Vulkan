#include "pch.h"
#include "BirdObject.h"

BirdObject::BirdObject(VulkanCore* core)
	:QuadObj(core)
{
	Texture.reset(new VulkanTexture2D("assets/textures/flappyBird/bird.png", core));

	SetScale(0.5);
}

BirdObject::~BirdObject()
{
}

void BirdObject::OnUpdate(float deltaTime)
{
}
