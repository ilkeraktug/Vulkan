#include "pch.h"
#include "VMaterialD.h"

#include "Vulkan/Renderer/V2/MaterialIDRenderer.h"

VMaterialD::VMaterialD()
{
    ID = MaterialIDRenderer::GlobalID++;
}
