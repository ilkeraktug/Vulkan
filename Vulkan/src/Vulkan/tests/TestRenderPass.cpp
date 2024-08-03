#include "pch.h"
#include "TestRenderPass.h"

#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/Renderer/V2/VulkanFrameBuffer.h"
#include "Vulkan/Renderer/V2/VulkanUniformBuffer2.h"

namespace test
{
    TestRenderPass::TestRenderPass(VulkanCore* core)
    {
        Init(core);
        m_Camera = std::make_unique<PerspectiveCamera>(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
        m_Camera->setPosition(glm::vec3(-5.0f, 0.0f, -5.0f));

        runBatchFile();
        loadassets();
        deferredSetup();
        shadowSetup();
        prepareUniformBuffers();
        prepareDescriptorSetLayout();
        preparePipeline();
        writeDescriptors();
        buildDeferredCommandBuffers();
        
        init = true;
    }

    TestRenderPass::~TestRenderPass()
    {
    }

    void TestRenderPass::OnUpdate(float deltaTime)
    {
        if(!init)
        {
            return;
        }
        
        m_Camera->OnUpdate(deltaTime);

        timer += timerSpeed * deltaTime;
        if (timer > 1.0f)
        {
            timer -= 1.0f;
        }

        updateUniformBuffers();
    }

    void TestRenderPass::OnRender()
    {
        if (!init)
        {
            return;
        }

        m_Core->BeginScene();

        m_Core->resources.submitInfo.commandBufferCount = 1;
        m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

        buildDeferredCommandBuffers();

        VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

        VkResult err = m_Core->Submit();

        //TODO : Fences and Semaphores !
        vkDeviceWaitIdle(m_Core->GetDevice());
    }

    void TestRenderPass::OnImGuiRender()
    {
    }

    void TestRenderPass::runBatchFile()
    {
        std::system(
        "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tps_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
        std::system(
        "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for /r %i in (*.geom) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tgs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.geom.spv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rchit.spv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rmiss.spv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain   -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing  -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rgen.spv)");
    }

    void TestRenderPass::loadassets()
    {
        std::string AssetsPath = "C:\\dev\\Vulkan\\Vulkan\\assets\\";
        const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
        models.model.loadFromFile(AssetsPath + "models/armor/armor.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
        models.background.loadFromFile(AssetsPath + "models/deferred_box.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
        textures.model.colorMap.loadFromFile(AssetsPath+ "models/armor/colormap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.model.normalMap.loadFromFile(AssetsPath + "models/armor/normalmap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.background.colorMap.loadFromFile(AssetsPath + "textures/stonefloor02_color_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.background.normalMap.loadFromFile(AssetsPath + "textures/stonefloor02_normal_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
    }

    void TestRenderPass::deferredSetup()
    {
        scene.FrameBuffer = std::make_unique<VulkanFrameBuffer>(m_Core);
        scene.FrameBuffer->AddAttachment(scene.ColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->AddAttachment(scene.ColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->AddAttachment(scene.ColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->Create();

#if 0
        // VkAttachmentDescription colorAttachmentDescription{};
        // colorAttachmentDescription.format = scene.ColorFormat;
        // colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        // colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        // colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        //
        // VkAttachmentDescription depthAttachmentDescription{};
        // depthAttachmentDescription.format = scene.DepthFormat;
        // depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        // depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        //
        // std::array<VkAttachmentDescription, 2> attachmentsDescriptions
        // {
        //     colorAttachmentDescription, depthAttachmentDescription
        // };
        //
        // VkAttachmentReference colorAttachmentReference{};
        // colorAttachmentReference.attachment = 0;
        // colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        //
        // VkAttachmentReference depthAttachmentReference{};
        // depthAttachmentReference.attachment = 1;
        // depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        //
        // VkSubpassDescription subpassDescription{};
        // subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // subpassDescription.colorAttachmentCount = 1;
        // subpassDescription.pColorAttachments = &colorAttachmentReference;
        // subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
        //
        // std::array<VkSubpassDependency, 2> subpassDependicies{};
        // subpassDependicies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        // subpassDependicies[0].dstSubpass = 0;
        // subpassDependicies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // subpassDependicies[0].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        // subpassDependicies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // subpassDependicies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        // subpassDependicies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        //
        // subpassDependicies[1].srcSubpass = 0;
        // subpassDependicies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        // subpassDependicies[1].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        // subpassDependicies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // subpassDependicies[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        // subpassDependicies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // subpassDependicies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        //
        // VkRenderPassCreateInfo renderpassCI{};
        // renderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        // renderpassCI.attachmentCount = attachmentsDescriptions.size();
        // renderpassCI.pAttachments = attachmentsDescriptions.data();
        // renderpassCI.subpassCount = 1;
        // renderpassCI.pSubpasses = &subpassDescription;
        // renderpassCI.dependencyCount = subpassDependicies.size();
        // renderpassCI.pDependencies = subpassDependicies.data();
        //
        // VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderpassCI, nullptr, &scene.renderPass));
        //
        // VkImageCreateInfo colorImageCI{};
        // colorImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        // colorImageCI.imageType = VK_IMAGE_TYPE_2D;
        // colorImageCI.format = scene.ColorFormat;
        // colorImageCI.extent = scene.ColorExtent;
        // colorImageCI.mipLevels = 1;
        // colorImageCI.arrayLayers = 1;
        // colorImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        // colorImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        // colorImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // colorImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // colorImageCI.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        //
        // VK_CHECK(vkCreateImage(m_Core->GetDevice(), &colorImageCI, nullptr, &scene.colorImage));
        //
        // VkImageSubresourceRange imageSubresourceRange{};
        // imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // imageSubresourceRange.baseMipLevel = 0;
        // imageSubresourceRange.levelCount = 1;
        // imageSubresourceRange.baseArrayLayer = 0;
        // imageSubresourceRange.layerCount = 1;
        //
        // VkImageViewCreateInfo colorImageViewCI{};
        // colorImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // colorImageViewCI.image = scene.colorImage;
        // colorImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // colorImageViewCI.format = scene.ColorFormat;
        // colorImageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
        // colorImageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
        // colorImageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
        // colorImageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
        // colorImageViewCI.subresourceRange = imageSubresourceRange;
        //
        // VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &colorImageViewCI, nullptr, &scene.colorImageView));
        //
        // VkMemoryRequirements sceneAttachmentMemReq;
        //
        // vkGetImageMemoryRequirements(m_Core->GetDevice(), scene.colorImage, &sceneAttachmentMemReq);
        //
        // VkMemoryAllocateInfo sceneColorAttachMemAllocInfo = init::memAllocInfo();
        // sceneColorAttachMemAllocInfo.allocationSize = sceneAttachmentMemReq.size;
        // sceneColorAttachMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(sceneAttachmentMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &sceneColorAttachMemAllocInfo, nullptr, &scene.colorImageMemory));
        //
        // VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), scene.colorImage, scene.colorImageMemory, 0));
        //
        // VkImageCreateInfo depthImageCI{};
        // depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        // depthImageCI.imageType = VK_IMAGE_TYPE_2D;
        // depthImageCI.format = scene.DepthFormat;
        // depthImageCI.extent = scene.DepthExtent;
        // depthImageCI.mipLevels = 1;
        // depthImageCI.arrayLayers = 1;
        // depthImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        // depthImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        // depthImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        // depthImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // depthImageCI.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        //
        // VK_CHECK(vkCreateImage(m_Core->GetDevice(), &depthImageCI, nullptr, &scene.depthImage));
        //
        // imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        //
        // VkImageViewCreateInfo depthImageViewCI{};
        // depthImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // depthImageViewCI.image = scene.depthImage;
        // depthImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // depthImageViewCI.format = scene.DepthFormat;
        // depthImageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
        // depthImageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
        // depthImageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
        // depthImageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
        // depthImageViewCI.subresourceRange = imageSubresourceRange;
        //
        // VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &colorImageViewCI, nullptr, &scene.depthImageView));
        //
        // vkGetImageMemoryRequirements(m_Core->GetDevice(), scene.colorImage, &sceneAttachmentMemReq);
        //
        // VkMemoryAllocateInfo sceneDepthAttachMemAllocInfo = init::memAllocInfo();
        // sceneDepthAttachMemAllocInfo.allocationSize = sceneAttachmentMemReq.size;
        // sceneDepthAttachMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(sceneAttachmentMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &sceneDepthAttachMemAllocInfo, nullptr, &scene.depthImageMemory));
        //
        // VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), scene.depthImage, scene.depthImageMemory, 0));
        //
        // std::array<VkImageView, 2> attachments =
        // {
        //     scene.colorImageView, scene.depthImageView
        // };
        //
        // VkFramebufferCreateInfo frameBufferCI{};
        // frameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // frameBufferCI.renderPass = scene.renderPass;
        // frameBufferCI.attachmentCount = attachments.size();
        // frameBufferCI.pAttachments = attachments.data();
        // frameBufferCI.width = scene.ColorExtent.width;
        // frameBufferCI.height = scene.ColorExtent.height;
        // frameBufferCI.layers = 1;
        //
        // VK_CHECK(vkCreateFramebuffer(m_Core->GetDevice(), &frameBufferCI, nullptr, &scene.FrameBuffer));
#endif
    }

    void TestRenderPass::shadowSetup()
    {
        shadow.FrameBuffer = std::make_unique<VulkanFrameBuffer>(m_Core);
        shadow.FrameBuffer->AddAttachment(shadow.Format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, shadow.Extent.width, shadow.Extent.height, 3);
        shadow.FrameBuffer->Create();

#if 0
        std::vector<VkAttachmentDescription> framebufferAttachments;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = shadow.Format;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        framebufferAttachments.push_back(depthAttachment);

        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 0;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

        std::array<VkSubpassDependency, 2> subpassDependencies;
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    
        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount = framebufferAttachments.size();
        renderPassCI.pAttachments = framebufferAttachments.data();
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDescription;
        renderPassCI.dependencyCount = subpassDependencies.size();
        renderPassCI.pDependencies = subpassDependencies.data();

        VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderPassCI, nullptr, &shadow.renderPass));
    
        VkImageCreateInfo depthAttachmentImageCI{};
        depthAttachmentImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthAttachmentImageCI.imageType = VK_IMAGE_TYPE_2D;
        depthAttachmentImageCI.format = shadow.Format;
        depthAttachmentImageCI.extent = shadow.Extent;
        depthAttachmentImageCI.mipLevels = 1;
        depthAttachmentImageCI.arrayLayers = shadow.LightCount;
        depthAttachmentImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthAttachmentImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depthAttachmentImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depthAttachmentImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(m_Core->GetDevice(), &depthAttachmentImageCI, nullptr, &shadow.depthImage));

        VkMemoryRequirements depthImageMemReqs{};
    
        vkGetImageMemoryRequirements(m_Core->GetDevice(), shadow.depthImage, &depthImageMemReqs);
    
        VkMemoryAllocateInfo depthImageMemAllocInfo{};
        depthImageMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        depthImageMemAllocInfo.allocationSize = depthImageMemReqs.size;
        depthImageMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(depthImageMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
        VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &depthImageMemAllocInfo, nullptr, &shadow.depthImageMemory));

        VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), shadow.depthImage, shadow.depthImageMemory, 0));
    
        VkImageViewCreateInfo depthAttachmentImageViewCI{};
        depthAttachmentImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthAttachmentImageViewCI.image = shadow.depthImage;
        depthAttachmentImageViewCI.viewType = shadow.LightCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        depthAttachmentImageViewCI.format = shadow.Format;
        depthAttachmentImageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        depthAttachmentImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthAttachmentImageViewCI.subresourceRange.baseMipLevel = 0;
        depthAttachmentImageViewCI.subresourceRange.levelCount = 1;
        depthAttachmentImageViewCI.subresourceRange.baseArrayLayer = 0;
        depthAttachmentImageViewCI.subresourceRange.layerCount = shadow.LightCount;

        VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &depthAttachmentImageViewCI, nullptr, &shadow.depthImageView));
    
        VkFramebufferCreateInfo framebufferCI{};
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass = shadow.renderPass;
        framebufferCI.attachmentCount = 1;
        framebufferCI.pAttachments = &shadow.depthImageView;
        framebufferCI.width = shadow.Extent.width;
        framebufferCI.height = shadow.Extent.height;
        framebufferCI.layers = shadow.Extent.depth;
    
        VK_CHECK(vkCreateFramebuffer(m_Core->GetDevice(), &framebufferCI, nullptr, &shadow.FrameBuffer));
#endif
    }

    void TestRenderPass::prepareDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr  },
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
        };
        
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.bindingCount = descriptorSetLayoutBindings.size();
        descriptorSetLayoutCI.pBindings = descriptorSetLayoutBindings.data();
        
        VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;

        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));
    }

    void TestRenderPass::writeDescriptors()
    {
        std::vector<VkDescriptorPoolSize> descriptorPools =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
        };
        
        VkDescriptorPoolCreateInfo descriptorPoolCI{};
        descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCI.maxSets = 12;
        descriptorPoolCI.poolSizeCount = 2;
        descriptorPoolCI.pPoolSizes = descriptorPools.data();

        VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_DescriptorPool));
        
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_DescriptorSetLayout;
        
        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &m_DescriptorSet));

        VkDescriptorBufferInfo binding0Info;
        binding0Info.buffer = shadowUniformBuffer->GetHandle();
        binding0Info.offset = 0;
        binding0Info.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo binding0gBufferInfo;
        binding0gBufferInfo.buffer = gBufferUniformBuffer->GetHandle();
        binding0gBufferInfo.offset = 0;
        binding0gBufferInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo binding4Info;
        binding4Info.buffer = sceneUniformBuffer->GetHandle();
        binding4Info.offset = 0;
        binding4Info.range = VK_WHOLE_SIZE;

        VkDescriptorImageInfo binding1Info;
        binding1Info.sampler = scene.FrameBuffer->GetSampler();
        binding1Info.imageView = scene.FrameBuffer->GetImageView(0);
        binding1Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo binding2Info;
        binding2Info.sampler = scene.FrameBuffer->GetSampler();
        binding2Info.imageView = scene.FrameBuffer->GetImageView(1);
        binding2Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo binding3Info;
        binding3Info.sampler = scene.FrameBuffer->GetSampler();
        binding3Info.imageView = scene.FrameBuffer->GetImageView(2);
        binding3Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo binding5Info;
        binding5Info.sampler = shadow.FrameBuffer->GetSampler();
        binding5Info.imageView = shadow.FrameBuffer->GetImageView(0);
        binding5Info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        

        std::vector<VkWriteDescriptorSet> descriptorSetWrites =
        {
            init::getImageWrite(m_DescriptorSet, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding1Info),
            init::getImageWrite(m_DescriptorSet, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding2Info),
            init::getImageWrite(m_DescriptorSet, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding3Info),
            init::getBufferWrite(m_DescriptorSet, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding4Info),
            init::getImageWrite(m_DescriptorSet, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding5Info),
        };
        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites.size(), descriptorSetWrites.data(), 0, nullptr);


        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.model));
        std::vector<VkWriteDescriptorSet> descriptorSetWrites1 =
        {
            init::getBufferWrite(descriptorSets.model, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding0gBufferInfo),
            init::getImageWrite(descriptorSets.model, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.model.colorMap.descriptor),
            init::getImageWrite(descriptorSets.model, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.model.normalMap.descriptor),
        };

        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites1.size(), descriptorSetWrites1.data(), 0, nullptr);
        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.background));

        std::vector<VkWriteDescriptorSet> descriptorSetWrites2 =
        {
            init::getBufferWrite(descriptorSets.background, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding0gBufferInfo),
            init::getImageWrite(descriptorSets.background, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.background.colorMap.descriptor),
            init::getImageWrite(descriptorSets.background, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.background.normalMap.descriptor),
        };
        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites2.size(), descriptorSetWrites2.data(), 0, nullptr);
        

        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.shadow));
        std::vector<VkWriteDescriptorSet> descriptorSetWrites3 =
        {
            init::getBufferWrite(descriptorSets.shadow, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding0Info),
        };

        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites3.size(), descriptorSetWrites3.data(), 0, nullptr);
    }

    void TestRenderPass::preparePipeline()
    {
        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/shadow.vspv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/shadow.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);

        std::vector<VkVertexInputBindingDescription> shadowVertexShaderInputBindings =
        {
            {0, 96, VK_VERTEX_INPUT_RATE_VERTEX}
        };



        std::vector<VkVertexInputAttributeDescription> shadowVertexShaderInputAttributes =
        {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, pos) },
            {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vkglTF::Vertex, uv)},
            {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, color)},
            {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, normal)},
            {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, tangent)},
        };
        

        
        VkPipelineVertexInputStateCreateInfo shadowVertexInputStateCI{};
        shadowVertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        shadowVertexInputStateCI.vertexBindingDescriptionCount = shadowVertexShaderInputBindings.size();
        shadowVertexInputStateCI.pVertexBindingDescriptions = shadowVertexShaderInputBindings.data();
        shadowVertexInputStateCI.vertexAttributeDescriptionCount = shadowVertexShaderInputAttributes.size();
        shadowVertexInputStateCI.pVertexAttributeDescriptions = shadowVertexShaderInputAttributes.data();

        VkPipelineInputAssemblyStateCreateInfo shadowInputAssemblyCI{};
        shadowInputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        shadowInputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        shadowInputAssemblyCI.primitiveRestartEnable = false;

        VkViewport shadowViewport{0, 0, static_cast<float>(shadow.Extent.width), static_cast<float>(shadow.Extent.height), 0, 1.0f};
        
        VkPipelineViewportStateCreateInfo shadowInputCI{};
        shadowInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        shadowInputCI.viewportCount = 1;
        shadowInputCI.pViewports = &shadowViewport;

        VkPipelineRasterizationStateCreateInfo shadowRasterizationStateCI{};
        shadowRasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        shadowRasterizationStateCI.depthClampEnable = VK_TRUE;
        shadowRasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
        shadowRasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        shadowRasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
        shadowRasterizationStateCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
        shadowRasterizationStateCI.depthBiasEnable = VK_TRUE;
        shadowRasterizationStateCI.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo shadowMultisampleStateCI{};
        shadowMultisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        shadowMultisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        shadowMultisampleStateCI.sampleShadingEnable = VK_FALSE;
        shadowMultisampleStateCI.alphaToCoverageEnable = VK_FALSE;
        shadowMultisampleStateCI.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo shadowDepthStencilStateCI{};
        shadowDepthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        shadowDepthStencilStateCI.depthTestEnable = VK_FALSE;
        shadowDepthStencilStateCI.depthWriteEnable = VK_TRUE;
        shadowDepthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        shadowDepthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
        shadowDepthStencilStateCI.stencilTestEnable = VK_FALSE;
        shadowDepthStencilStateCI.minDepthBounds = 0.0f;
        shadowDepthStencilStateCI.maxDepthBounds = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.blendEnable = VK_FALSE;
        blendAttachmentState.colorWriteMask = 0xf;
        
        VkPipelineColorBlendStateCreateInfo  colorBlendStateCI{};
        colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCI.attachmentCount = 0;
        colorBlendStateCI.pAttachments = nullptr;

        std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS};
        
        VkPipelineDynamicStateCreateInfo shadowDynamicStateCI{};
        shadowDynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        shadowDynamicStateCI.dynamicStateCount = dynamicStateEnables.size();
        shadowDynamicStateCI.pDynamicStates = dynamicStateEnables.data();

        VkViewport viewPortShadow = { 0, 0, (float)shadow.Extent.width, (float)shadow.Extent.height, 0.0f, 1.0f };
        
        VkRect2D scissorShadow;
        scissorShadow.offset = { 0, 0 };
        scissorShadow.extent = { shadow.Extent.width, shadow.Extent.height };


        VkPipelineViewportStateCreateInfo viewportState = init::pipelineViewportState();
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissorShadow;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewPortShadow;
        
        VkGraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.stageCount = 2;
        pipelineCI.pStages = shaderStages; 
        pipelineCI.pVertexInputState = &shadowVertexInputStateCI;
        pipelineCI.pInputAssemblyState = &shadowInputAssemblyCI;
        pipelineCI.pTessellationState = nullptr;
        pipelineCI.pViewportState = &viewportState;
        pipelineCI.pRasterizationState = &shadowRasterizationStateCI;
        pipelineCI.pMultisampleState = &shadowMultisampleStateCI;
        pipelineCI.pDepthStencilState = &shadowDepthStencilStateCI;
        pipelineCI.pColorBlendState = &colorBlendStateCI;
        pipelineCI.pDynamicState = &shadowDynamicStateCI;
        pipelineCI.layout = m_PipelineLayout;
        pipelineCI.renderPass = shadow.FrameBuffer->GetRenderPass();

        VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipelines.shadow));

        shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/scene.vspv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/scene.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);

        viewPortShadow = { 0, 0, (float)m_Core->swapchain.extent.width, (float)m_Core->swapchain.extent.height, 0.0f, 1.0f };
        
        scissorShadow.offset = { 0, 0 };
        scissorShadow.extent = { m_Core->swapchain.extent.width, m_Core->swapchain.extent.height };
        
        VkPipelineVertexInputStateCreateInfo emptyVertexInput{};
        emptyVertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        emptyVertexInput.vertexBindingDescriptionCount = 0;
        emptyVertexInput.pVertexBindingDescriptions = nullptr;
        emptyVertexInput.vertexAttributeDescriptionCount = 0;
        emptyVertexInput.pVertexAttributeDescriptions = nullptr;

        pipelineCI.pVertexInputState = &emptyVertexInput;
        pipelineCI.renderPass = m_Core->resources.renderPass;


        //
        // std::vector<VkPipelineColorBlendAttachmentState> sceneColorAttachments =
        // {
        //     noBlendColorAttachments
        // };
        //
        // VkPipelineColorBlendStateCreateInfo sceneColorBlendState{};
        // sceneColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // sceneColorBlendState.logicOpEnable = VK_FALSE;
        // sceneColorBlendState.attachmentCount = sceneColorAttachments.size();
        // sceneColorBlendState.pAttachments = sceneColorAttachments.data();
        //
        // pipelineCI.pColorBlendState = &sceneColorBlendState;

        VkPipelineColorBlendAttachmentState noBlendColorAttachments{};
        noBlendColorAttachments.blendEnable = VK_FALSE;
        noBlendColorAttachments.colorWriteMask = 0xf;
        
        colorBlendStateCI.attachmentCount = 1;
        colorBlendStateCI.pAttachments = &noBlendColorAttachments;

        VkViewport sceneViewport{0, 0, static_cast<float>(scene.ColorExtent.width), static_cast<float>(scene.ColorExtent.height), 0, 1.0f};
        
        VkPipelineViewportStateCreateInfo sceneViewportCI{};
        sceneViewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        sceneViewportCI.viewportCount = 1;
        sceneViewportCI.pViewports = &sceneViewport;
        
        VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipelines.scene));

        shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/mrt.vspv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/mrt.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);

        viewPortShadow = { 0, 0, (float)scene.ColorExtent.width, (float)scene.ColorExtent.height, 0.0f, 1.0f };
        
        scissorShadow.offset = { 0, 0 };
        scissorShadow.extent = { scene.ColorExtent.width, scene.ColorExtent.height };
        

        
        std::vector<VkPipelineColorBlendAttachmentState> gBufferColorAttachments =
        {
            noBlendColorAttachments, noBlendColorAttachments, noBlendColorAttachments
        };
        
        VkPipelineColorBlendStateCreateInfo gBufferColorBlendState{};
        gBufferColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        gBufferColorBlendState.logicOpEnable = VK_FALSE;
        gBufferColorBlendState.attachmentCount = gBufferColorAttachments.size();
        gBufferColorBlendState.pAttachments = gBufferColorAttachments.data();

        pipelineCI.pColorBlendState = &gBufferColorBlendState;

        std::vector<VkVertexInputBindingDescription> gBufferVertexShaderInputBindings =
        {
                    {0, 96, VK_VERTEX_INPUT_RATE_VERTEX}
        };

        std::vector<VkVertexInputAttributeDescription> gBufferVertexShaderInputAttributes =
        {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, pos) },
            {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vkglTF::Vertex, uv)},
            {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, color)},
            {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, normal)},
            {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, tangent)},
        }; 
        
        VkPipelineVertexInputStateCreateInfo gBufferVertexInputStateCI{};
        gBufferVertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        gBufferVertexInputStateCI.vertexBindingDescriptionCount = gBufferVertexShaderInputBindings.size();
        gBufferVertexInputStateCI.pVertexBindingDescriptions = gBufferVertexShaderInputBindings.data();
        gBufferVertexInputStateCI.vertexAttributeDescriptionCount = gBufferVertexShaderInputAttributes.size();
        gBufferVertexInputStateCI.pVertexAttributeDescriptions = gBufferVertexShaderInputAttributes.data();

        
        pipelineCI.pVertexInputState = &gBufferVertexInputStateCI;
        pipelineCI.renderPass = scene.FrameBuffer->GetRenderPass();
        
        VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipelines.gBuffer));
    }

    void TestRenderPass::prepareUniformBuffers()
    {
        SceneUniformBufferStructObject.lights[0].Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        SceneUniformBufferStructObject.lights[1].Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        SceneUniformBufferStructObject.lights[2].Color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

        SceneUniformBufferStructObject.lights[0].Position = glm::vec4(-14.0f, -0.5f, 15.0f, 1.0f);
        SceneUniformBufferStructObject.lights[1].Position = glm::vec4(14.0f, -4.0f, 12.0f, 1.0f);
        SceneUniformBufferStructObject.lights[2].Position = glm::vec4(0.0f, -10.0f, 4.0f, 1.0f);

        SceneUniformBufferStructObject.lights[0].Target = glm::vec4(-2.0f, 0.0f, 0.0f, 1.0f);
        SceneUniformBufferStructObject.lights[1].Target = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
        SceneUniformBufferStructObject.lights[2].Target = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        SceneUniformBufferStructObject.debugShadow = 0;
        
        gBufferUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(gBufferUniformBufferStruct));
        shadowUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(ShadowUniformBufferStruct));
        sceneUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(SceneUniformBufferStruct));
    }

    void TestRenderPass::updateUniformBuffers()
    {
        SceneUniformBufferStructObject.lights[0].Position.x = -14.0f + std::abs(sin(glm::radians(timer * 360.0f)) * 20.0f);
        SceneUniformBufferStructObject.lights[0].Position.z = 15.0f + cos(glm::radians(timer *360.0f)) * 1.0f;
        
        SceneUniformBufferStructObject.lights[1].Position.x = 14.0f - std::abs(sin(glm::radians(timer * 360.0f)) * 2.5f);
        SceneUniformBufferStructObject.lights[1].Position.z = 13.0f + cos(glm::radians(timer *360.0f)) * 4.0f;
        
        SceneUniformBufferStructObject.lights[2].Position.x = 0.0f + sin(glm::radians(timer *360.0f)) * 4.0f;
        SceneUniformBufferStructObject.lights[2].Position.z = 4.0f + cos(glm::radians(timer *360.0f)) * 2.0f;

        for (uint32_t i = 0; i < 3; i++)
        {
            // mvp from light's pov (for shadows)
            glm::mat4 shadowProj = glm::perspective(glm::radians(100.0f), 1.0f, 0.1f, 100.0f);
            glm::mat4 shadowView = glm::lookAt(glm::vec3(SceneUniformBufferStructObject.lights[i].Position), glm::vec3(SceneUniformBufferStructObject.lights[i].Target), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 shadowModel = glm::mat4(1.0f);
            
            ShadowUniformBufferStructObject.MVPs[i] = shadowProj * shadowView * shadowModel;
            ShadowUniformBufferStructObject.InstancePos[0] = glm::vec4(0.0f);
            ShadowUniformBufferStructObject.InstancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
            ShadowUniformBufferStructObject.InstancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);
            
            SceneUniformBufferStructObject.lights[i].MVP = shadowProj * shadowView * shadowModel;
        }

        SceneUniformBufferStructObject.viewPosition = glm::vec4(m_Camera->getPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

        gBufferUniformBufferStructObject.InstancePos[0] = glm::vec4(0.0f);
        gBufferUniformBufferStructObject.InstancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
        gBufferUniformBufferStructObject.InstancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);

        gBufferUniformBufferStructObject.Model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        gBufferUniformBufferStructObject.View = m_Camera->getViewMatrix();
        gBufferUniformBufferStructObject.Projection = m_Camera->getProjectionMatrix();
        
        gBufferUniformBuffer->copyToBuffer(&gBufferUniformBufferStructObject, sizeof(gBufferUniformBufferStructObject));
        shadowUniformBuffer->copyToBuffer(&ShadowUniformBufferStructObject, sizeof(ShadowUniformBufferStructObject));
        sceneUniformBuffer->copyToBuffer(&SceneUniformBufferStructObject, sizeof(SceneUniformBufferStructObject));
    }

    void TestRenderPass::buildDeferredCommandBuffers()
    {
        VkClearValue clearValue{};
        clearValue.color = {{0.0f, 0.0f, 0.0f, 0.0f} };
        clearValue.depthStencil = { 1.0f, 0 };

        VkClearValue clearValue2{};
        clearValue2.color.float32[0] = 0.0f;
        clearValue2.color.float32[1] = 0.0f;
        clearValue2.color.float32[2] = 0.0f;
        clearValue2.color.float32[3] = 0.0f;
        
        clearValue2.depthStencil.depth = 1.0f;
        clearValue2.depthStencil.stencil = 0.0f;

        VkClearValue clearValue3{};
        
        clearValue3.depthStencil.depth = 1.0f;
        clearValue3.depthStencil.stencil = 0.0f;

        std::vector<VkClearValue> sceneClearValues =
        {
            clearValue2, clearValue3
        };

        VkRect2D scissor;
        
        VkRenderPassBeginInfo renderPassBI = init::renderPassBeginInfo();
        renderPassBI.clearValueCount = 1;
        renderPassBI.pClearValues = &clearValue;
        
        std::array<VkClearValue, 4> clearValues = {};
        
        VkCommandBufferBeginInfo cmdBufferBI = init::commandBufferBeginInfo();

        for(int i = 0; i < 3; i++)
        {
            VkCommandBuffer& currentCmdBuffer = m_Core->resources.drawCmdBuffers[i];

            renderPassBI.framebuffer = shadow.FrameBuffer->GetHandle();
            renderPassBI.renderPass = shadow.FrameBuffer->GetRenderPass();
            renderPassBI.renderArea = { {0, 0 }, { shadow.Extent.width, shadow.Extent.height } };

            VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBufferBI));
            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdSetDepthBias(
            currentCmdBuffer,
            1.25f,
            0.0f,
            1.75f);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = shadow.Extent.width;
            viewport.height = shadow.Extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            vkCmdSetViewport(currentCmdBuffer, 0, 1, &viewport);
            
            scissor.offset = {0, 0};
            scissor.extent = { shadow.Extent.width, shadow.Extent.height };
            
            vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.shadow);
            // Background
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.shadow, 0, NULL);
            models.background.draw(currentCmdBuffer);

            // Objects
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.shadow, 0, NULL);
            models.model.bindBuffers(currentCmdBuffer);
            vkCmdDrawIndexed(currentCmdBuffer, models.model.indices.count, 3, 0, 0, 0);
            

            vkCmdEndRenderPass(currentCmdBuffer);
            
            // Clear values for all attachments written in the fragment shader
            clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[3].depthStencil = { 1.0f, 0 };


            renderPassBI.renderPass = scene.FrameBuffer->GetRenderPass();
            renderPassBI.framebuffer = scene.FrameBuffer->GetHandle();
            renderPassBI.renderArea.extent.width = scene.ColorExtent.width;
            renderPassBI.renderArea.extent.height = scene.ColorExtent.height;
            renderPassBI.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassBI.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
            
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = scene.ColorExtent.width;
            viewport.height = scene.ColorExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            vkCmdSetViewport(currentCmdBuffer, 0, 1, &viewport);
            
            scissor.offset = {0, 0};
            scissor.extent = { scene.ColorExtent.width, scene.ColorExtent.height };
            
            vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.gBuffer);
            // Background
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.background, 0, NULL);
            models.background.draw(currentCmdBuffer);

            // Objects
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.model, 0, NULL);
            models.model.bindBuffers(currentCmdBuffer);
            vkCmdDrawIndexed(currentCmdBuffer, models.model.indices.count, 3, 0, 0, 0);
            
            vkCmdEndRenderPass(currentCmdBuffer);

            renderPassBI.renderPass = m_Core->resources.renderPass;
            renderPassBI.framebuffer = m_Core->resources.frameBuffers[i];
            renderPassBI.renderArea.extent.width = m_Core->swapchain.extent.width;
            renderPassBI.renderArea.extent.height = m_Core->swapchain.extent.height;
            renderPassBI.clearValueCount = 2;
            renderPassBI.pClearValues = sceneClearValues.data();

            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);

            // Final composition as full screen quad
            // Note: Also used for debug display if debugDisplayTarget > 0
            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.scene);
            vkCmdDraw(currentCmdBuffer, 3, 1, 0, 0);
            
            vkCmdEndRenderPass(currentCmdBuffer);
            VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));
        }
        
    }
}
