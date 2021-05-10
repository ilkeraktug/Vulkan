#pragma once

#include "Drawable.h"


class QuadObj : public Drawable
{
public:
	QuadObj() {};
	QuadObj(VulkanCore* core);

	virtual ~QuadObj();

private:
};